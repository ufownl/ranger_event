#include <event/dispatcher.hpp>
#include <event/tcp_connection.hpp>
#include <event/buffer.hpp>
#include <iostream>

class simple_client : public ranger::event::tcp_connection::event_handler
{
public:
	simple_client(const char* addr, int port)
		: m_conn(ranger::event::tcp_connection::create(m_disp, addr, port))
	{
		m_conn->set_event_handler(this);
	}

	int run()
	{
		char content[] = "GET / HTTP/1.1\r\n\r\n";
		m_conn->send(content, strlen(content));
		return m_disp.run();
	}

private:
	void handle_read(ranger::event::tcp_connection& conn, ranger::event::buffer&& buf) final
	{
		while (buf.size() > 0)
		{
			std::string line = buf.readln();
			if (!line.empty()) std::cout << line << std::endl;
		}
	}

	void handle_connected(ranger::event::tcp_connection& conn) final
	{
		auto ep = conn.remote_endpoint();
		std::cout << "connection[" << ep << "] connected." << std::endl;
	}

	void handle_error(ranger::event::tcp_connection& conn) final
	{
		auto ep = conn.remote_endpoint();
		std::cerr << "connection[" << ep << "] " << "error[" << conn.error_code() << "]: " << conn.error_description() << std::endl;
		conn.close();
	}

private:
	ranger::event::dispatcher m_disp;
	std::shared_ptr<ranger::event::tcp_connection> m_conn;
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
