#include <event/dispatcher.hpp>
#include <event/tcp_connection.hpp>
#include <event/buffer.hpp>
#include <string.h>
#include <iostream>
#include <vector>

class simple_client : public ranger::event::tcp_connection::event_handler
{
public:
	simple_client(const char* addr, int port)
		: m_conn(m_disp, addr, port)
	{
		m_conn.set_event_handler([this] (ranger::event::tcp_connection& conn, ranger::event::tcp_connection::event_code what)
			{
				switch (what)
				{
				case ranger::event::tcp_connection::event_code::read:
					handle_read(conn, conn.read_buffer());
					break;
				case ranger::event::tcp_connection::event_code::connected:
					handle_connected(conn);
					break;
				case ranger::event::tcp_connection::event_code::error:
					handle_error(conn);
					break;
				default:
					break;
				}
			});
	}

	int run()
	{
		char content[] = "GET / HTTP/1.1\r\n\r\n";
		m_conn.write_buffer().append(content, strlen(content));
		return m_disp.run();
	}

private:
	void handle_read(ranger::event::tcp_connection& conn, ranger::event::buffer&& buf)
	{
		std::vector<char> v(buf.size());
		buf.remove(&v.front(), v.size());
		for (auto ch: v) std::cout << ch << std::flush;
	}

	void handle_connected(ranger::event::tcp_connection& conn)
	{
		auto ep = conn.remote_endpoint();
		std::cout << "connection[" << ep << "] connected." << std::endl;
	}

	void handle_error(ranger::event::tcp_connection& conn)
	{
		auto ep = conn.remote_endpoint();
		std::cerr << "connection[" << ep << "] " << "error[" << conn.error_code() << "]: " << conn.error_description() << std::endl;
		conn.close();
	}

private:
	ranger::event::dispatcher m_disp;
	ranger::event::tcp_connection m_conn;
};

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: simple_client <address> <port>" << std::endl;
		return -1;
	}

	return simple_client(argv[1], atoi(argv[2])).run();
}
