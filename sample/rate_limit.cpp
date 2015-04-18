#include <event/dispatcher.hpp>
#include <event/tcp_connection.hpp>
#include <event/buffer.hpp>
#include <iostream>
#include <vector>

class rate_limit : public ranger::event::tcp_connection::event_handler
{
public:
	rate_limit(const char* addr, int port)
		: m_conn(ranger::event::tcp_connection::create(m_disp, addr, port))
	{
		m_conn->set_event_handler(this);
		m_conn->set_rate_limit(*ranger::event::token_bucket_cfg::create(10, 50, INT_MAX, INT_MAX, 0.2f));
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
		std::vector<char> v;
		while (buf.size() > 0)
		{
			v.resize(buf.size());
			buf.remove(&v.front(), v.size());
			
			for (auto ch: v) std::cout << ch << std::flush;
		}
	}

	void handle_connected(ranger::event::tcp_connection& conn) final
	{
		auto ep = conn.remote_endpoint();
		std::cout << "connection[" << ep.addr() << ":" << ep.port() << "] connected." << std::endl;
	}

	void handle_error(ranger::event::tcp_connection& conn) final
	{
		auto ep = conn.remote_endpoint();
		std::cerr << "connection[" << ep.addr() << ":" << ep.port() << "] " << "error[" << conn.error_code() << "]: " << conn.error_description() << std::endl;
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
		std::cerr << "Usage: rate_limit <address> <port>" << std::endl;
		return -1;
	}

	return rate_limit(argv[1], atoi(argv[2])).run();
}
