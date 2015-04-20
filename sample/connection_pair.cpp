#include <event/dispatcher.hpp>
#include <event/tcp_connection.hpp>
#include <event/buffer.hpp>
#include <iostream>
#include <vector>

class connection_pair : public ranger::event::tcp_connection::event_handler
{
public:
	explicit connection_pair(size_t cnt)
		: m_cnt(cnt * 2)
	{
		m_conn_pair.first->set_event_handler(this);
		m_conn_pair.second->set_event_handler(this);
	}

	int run()
	{
		char msg[] = "Hello, world!\n";
		m_conn_pair.first->send(msg, strlen(msg));
		return m_disp.run();
	}

private:
	void handle_read(ranger::event::tcp_connection& conn, ranger::event::buffer&& buf) final
	{
		if (m_cnt-- > 0)
		{
			std::vector<char> v(buf.size());
			buf.remove(&v.front(), v.size());
			for (auto ch: v) std::cout << ch;
			conn.send(&v.front(), v.size());
		}
		else
		{
			m_conn_pair.first.reset();
			m_conn_pair.second.reset();
		}
	}

private:
	ranger::event::dispatcher m_disp;
	std::pair<
		std::shared_ptr<ranger::event::tcp_connection>,
		std::shared_ptr<ranger::event::tcp_connection>
	> m_conn_pair = ranger::event::tcp_connection::create_pair(m_disp);
	size_t m_cnt;
};

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: connection_pair <ping_pong_cnt>" << std::endl;
		return -1;
	}

	return connection_pair(atoi(argv[1])).run();
}
