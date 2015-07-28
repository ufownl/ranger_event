#include <event/dispatcher.hpp>
#include <event/tcp_connection.hpp>
#include <event/buffer.hpp>
#include <string.h>
#include <iostream>
#include <vector>

class connection_pair {
public:
	explicit connection_pair(size_t cnt)
		: m_cnt(cnt * 2) {
		auto handler = [this] (ranger::event::tcp_connection& conn, ranger::event::tcp_connection::event_code what) {
			if (what == ranger::event::tcp_connection::event_code::read) {
				if (m_cnt-- > 0) {
					std::vector<char> v(conn.read_buffer().size());
					conn.read_buffer().remove(&v.front(), v.size());
					for (auto ch: v) std::cout << ch;
					conn.write_buffer().append(&v.front(), v.size());
				} else {
					m_conn_pair.first.close();
					m_conn_pair.second.close();
				}
			}
		};

		m_conn_pair.first.set_event_handler(handler);
		m_conn_pair.second.set_event_handler(handler);
	}

	int run() {
		char msg[] = "Hello, world!\n";
		m_conn_pair.first.write_buffer().append(msg, strlen(msg));
		return m_disp.run();
	}

private:
	ranger::event::dispatcher m_disp;
	std::pair<
		ranger::event::tcp_connection,
		ranger::event::tcp_connection
	> m_conn_pair = ranger::event::tcp_connection::create_pair(m_disp);
	size_t m_cnt;
};

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cerr << "Usage: connection_pair <ping_pong_cnt>" << std::endl;
		return -1;
	}

	return connection_pair(atoi(argv[1])).run();
}
