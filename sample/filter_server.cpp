#include <event/dispatcher.hpp>
#include <event/tcp_acceptor.hpp>
#include <event/tcp_connection.hpp>
#include <event/buffer.hpp>
#include <iostream>
#include <vector>
#include <set>

class size_filter : public ranger::event::tcp_connection::filter_handler {
public:
	explicit size_filter(size_t sz)
		: m_size(sz) {
		// nop
	}

	~size_filter() {
		std::cout << __FUNCTION__ << std::endl;
	}

	bool handle_input(ranger::event::buffer&& src, ranger::event::buffer&& dst) final {
		size_t n = src.size() / m_size;
		if (n == 0) {
			return false;
		}

		std::vector<char> v(n * m_size);
		src.remove(&v.front(), v.size());
		dst.append(&v.front(), v.size());
		return true;
	}

	bool handle_output(ranger::event::buffer&& src, ranger::event::buffer&& dst) final {
		dst.append(src);
		return true;
	}

private:
	size_t m_size;
};

class transform_filter : public ranger::event::tcp_connection::filter_handler {
private:
	~transform_filter() {
		std::cout << __FUNCTION__ << std::endl;
	}

	void transform(ranger::event::buffer& src, ranger::event::buffer& dst) {
		std::vector<char> v(src.size());
		src.remove(&v.front(), v.size());
		for (auto& ch: v)
			if ('a' <= ch && ch < 'z') ++ch;
		dst.append(&v.front(), v.size());
	}

	bool handle_input(ranger::event::buffer&& src, ranger::event::buffer&& dst) final {
		transform(src, dst);
		return true;
	}

	bool handle_output(ranger::event::buffer&& src, ranger::event::buffer&& dst) final {
		transform(src, dst);
		return true;
	}
};

class filter_server
	: public ranger::event::tcp_acceptor::event_handler
	, public ranger::event::tcp_connection::event_handler {
public:
	filter_server(int port, size_t sz)
		: m_acc(m_disp, ranger::event::endpoint(port))
		, m_size(sz) {
		m_acc.set_event_handler([this] (ranger::event::tcp_acceptor& acc, int fd) {
			ranger::event::tcp_connection conn(m_disp, fd);
			conn.append_filter<size_filter>(m_size);
			conn.append_filter<transform_filter>();
			conn.set_event_handler([this] (ranger::event::tcp_connection& conn, ranger::event::tcp_connection::event_code what) {
				switch (what) {
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

	int run() {
		return m_disp.run();
	}

private:
	void handle_read(ranger::event::tcp_connection& conn, ranger::event::buffer&& buf) {
		std::vector<char> v(buf.size());
		buf.remove(&v.front(), v.size());
		for (auto ch: v) std::cout << ch << std::flush;
		conn.write_buffer().append(&v.front(), v.size());
	}

	void handle_timeout(ranger::event::tcp_connection& conn) {
		auto ep = conn.remote_endpoint();
		std::cerr << "connection[" << ep << "] " << "timeout." << std::endl;

		m_conn_set.erase(conn);
	}

	void handle_error(ranger::event::tcp_connection& conn) {
		auto ep = conn.remote_endpoint();
		std::cerr << "connection[" << ep << "] " << "error[" << conn.error_code() << "]: " << conn.error_description() << std::endl;

		m_conn_set.erase(conn);
	}

	void handle_eof(ranger::event::tcp_connection& conn) {
		auto ep = conn.remote_endpoint();
		std::cerr << "connection[" << ep << "] " << "eof." << std::endl;

		m_conn_set.erase(conn);
	}

private:
	ranger::event::dispatcher m_disp;
	ranger::event::tcp_acceptor m_acc;
	size_t m_size;

	std::set<ranger::event::tcp_connection> m_conn_set;
};

int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cerr << "Usage: filter_server <port> <size>" << std::endl;
		return -1;
	}

	return filter_server(atoi(argv[1]), atoi(argv[2])).run();
}
