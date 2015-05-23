#include <event/dispatcher.hpp>
#include <event/tcp_acceptor.hpp>
#include <event/tcp_connection.hpp>
#include <event/buffer.hpp>
#include <iostream>
#include <set>

class echo_server
	: public ranger::event::tcp_acceptor::event_handler
	, public ranger::event::tcp_connection::event_handler
{
public:
	explicit echo_server(int port)
		: m_acc(m_disp, ranger::event::endpoint(port))
	{
		m_acc.set_event_handler([this] (ranger::event::tcp_acceptor& acc, int fd)
				{
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

					auto local_ep = acc.local_endpoint();
					auto remote_ep = conn.remote_endpoint();
					std::cout << "acceptor[" << local_ep << "]" << " accept connection[" << remote_ep << "]." << std::endl;

					m_conn_set.emplace(std::move(conn));

					return true;
				});
	}

	int run()
	{
		return m_disp.run();
	}

private:
	void handle_read(ranger::event::tcp_connection& conn, ranger::event::buffer&& buf)
	{
		conn.write_buffer().append(buf);
	}

	void handle_timeout(ranger::event::tcp_connection& conn)
	{
		auto ep = conn.remote_endpoint();
		std::cerr << "connection[" << ep << "] " << "timeout." << std::endl;

		m_conn_set.erase(conn);
	}

	void handle_error(ranger::event::tcp_connection& conn)
	{
		auto ep = conn.remote_endpoint();
		std::cerr << "connection[" << ep << "] " << "error[" << conn.error_code() << "]: " << conn.error_description() << std::endl;

		m_conn_set.erase(conn);
	}

	void handle_eof(ranger::event::tcp_connection& conn)
	{
		auto ep = conn.remote_endpoint();
		std::cerr << "connection[" << ep << "] " << "eof." << std::endl;

		m_conn_set.erase(conn);
	}

private:
	ranger::event::dispatcher m_disp;
	ranger::event::tcp_acceptor m_acc;

	std::set<ranger::event::tcp_connection> m_conn_set;
};

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: echo_server <port>" << std::endl;
		return -1;
	}

	return echo_server(atoi(argv[1])).run();
}
