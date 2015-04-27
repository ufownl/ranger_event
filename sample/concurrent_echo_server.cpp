#include <event/dispatcher.hpp>
#include <event/tcp_acceptor.hpp>
#include <event/tcp_connection.hpp>
#include <event/buffer.hpp>
#include <util/scope_guard.hpp>
#include <iostream>
#include <vector>
#include <unordered_set>
#include <thread>

class echo_server
{
public:
	explicit echo_server(ranger::event::dispatcher& disp)
		: m_disp(disp)
	{
	}

	void take_fd(int fd)
	{
		ranger::util::scope_guard fd_guard([fd] () { ranger::event::tcp_connection::file_descriptor_close(fd); });
		
		ranger::event::tcp_connection conn(m_disp, fd);
		conn.set_event_handler([this] (ranger::event::tcp_connection& conn, ranger::event::tcp_connection::event_code what)
				{
					switch (what)
					{
					case ranger::event::tcp_connection::event_code::read:
						handle_read(conn, conn.read_buffer());
						break;
					case ranger::event::tcp_connection::event_code::timeout:
						handle_timeout(conn);
						break;
					case ranger::event::tcp_connection::event_code::error:
						handle_error(conn);
						break;
					case ranger::event::tcp_connection::event_code::eof:
						handle_eof(conn);
						break;
					default:
						break;
					}
				});

		fd_guard.dismiss();

		auto remote_ep = conn.remote_endpoint();
		std::cout << "thread[" << std::this_thread::get_id() << "] " << "accept connection[" << remote_ep << "]." << std::endl;

		m_conn_set.emplace(std::move(conn));
	}

private:
	void handle_read(ranger::event::tcp_connection& conn, ranger::event::buffer&& buf)
	{
		std::cout << "thread[" << std::this_thread::get_id() << "] " << __FUNCTION__ << std::endl;
		conn.write_buffer().append(buf);
	}

	void handle_timeout(ranger::event::tcp_connection& conn)
	{
		auto ep = conn.remote_endpoint();
		std::cerr << "thread[" << std::this_thread::get_id() << "] " << "connection[" << ep << "] " << "timeout." << std::endl;

		m_conn_set.erase(conn);
	}

	void handle_error(ranger::event::tcp_connection& conn)
	{
		auto ep = conn.remote_endpoint();
		std::cerr << "thread[" << std::this_thread::get_id() << "] " << "connection[" << ep << "] " << "error[" << conn.error_code() << "]: " << conn.error_description() << std::endl;

		m_conn_set.erase(conn);
	}

	void handle_eof(ranger::event::tcp_connection& conn)
	{
		auto ep = conn.remote_endpoint();
		std::cerr << "thread[" << std::this_thread::get_id() << "] " << "connection[" << ep << "] " << "eof." << std::endl;

		m_conn_set.erase(conn);
	}

private:
	ranger::event::dispatcher& m_disp;
	std::unordered_set<ranger::event::tcp_connection> m_conn_set;
};

class worker
{
public:
	void run()
	{
		m_disp.run();
	}

	ranger::event::dispatcher& event_dispatcher() { return m_disp; }

	void set_external_connection(ranger::event::tcp_connection&& conn)
	{
		m_external_conn = std::move(conn);
	}

	void set_internal_connection(ranger::event::tcp_connection&& conn)
	{
		m_internal_conn = std::move(conn);
		m_internal_conn.set_event_handler([this] (ranger::event::tcp_connection& conn, ranger::event::tcp_connection::event_code what)
				{
					switch (what)
					{
					case ranger::event::tcp_connection::event_code::read:
						for (int fd = 0; conn.read_buffer().size() >= sizeof(fd); )
						{
							conn.read_buffer().remove(&fd, sizeof(fd));
							m_server.take_fd(fd);
						}
						break;
					case ranger::event::tcp_connection::event_code::timeout:
					case ranger::event::tcp_connection::event_code::error:
					case ranger::event::tcp_connection::event_code::eof:
						conn.close();
						break;
					default:
						break;
					}
				});
	}

	bool take_fd(int fd)
	{
		return m_external_conn.write_buffer().append(&fd, sizeof(fd));
	}

private:
	ranger::event::dispatcher m_disp;
	ranger::event::tcp_connection m_external_conn;
	ranger::event::tcp_connection m_internal_conn;
	echo_server m_server { m_disp };
};

class fd_dispatcher
	: public ranger::event::tcp_acceptor::event_handler
	, public ranger::event::tcp_connection::event_handler
{
public:
	fd_dispatcher(int port, size_t thread_cnt)
		: m_acc(m_disp, ranger::event::endpoint(port))
	{
		m_acc.set_event_handler([this] (ranger::event::tcp_acceptor& acc, int fd)
				{
					return m_workers[m_worker_idx++ % m_workers.size()].take_fd(fd);
				});

		decltype(m_workers) workers(thread_cnt);
		for (auto i = workers.begin(); i != workers.end(); ++i)
		{
			auto conn_pair = ranger::event::tcp_connection::create_pair(m_disp, i->event_dispatcher());
			conn_pair.first.set_event_handler([] (ranger::event::tcp_connection& conn, ranger::event::tcp_connection::event_code what)
					{
						switch (what)
						{
						case ranger::event::tcp_connection::event_code::timeout:
						case ranger::event::tcp_connection::event_code::error:
						case ranger::event::tcp_connection::event_code::eof:
							conn.close();
							break;
						default:
							break;
						}
					});

			i->set_external_connection(std::move(conn_pair.first));
			i->set_internal_connection(std::move(conn_pair.second));
		}

		m_workers = std::move(workers);
	}

	int run()
	{
		std::vector<std::thread> threads;
		threads.reserve(m_workers.size());
		for (auto& w: m_workers)
			threads.emplace_back(&worker::run, &w);

		int ret = m_disp.run();

		for (auto& t: threads) t.join();

		return ret;
	}

private:
	ranger::event::dispatcher m_disp;
	ranger::event::tcp_acceptor m_acc;
	std::vector<worker> m_workers;
	size_t m_worker_idx = 0;
};

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: concurrent_echo_server <port> <thread_cnt>" << std::endl;
		return -1;
	}
	
	return fd_dispatcher(atoi(argv[1]), atoi(argv[2])).run();
}
