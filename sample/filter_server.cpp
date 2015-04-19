#include <event/dispatcher.hpp>
#include <event/tcp_acceptor.hpp>
#include <event/tcp_connection.hpp>
#include <event/buffer.hpp>
#include <iostream>
#include <vector>
#include <map>

class size_filter : public ranger::event::tcp_connection::filter_handler
{
public:
	size_filter(size_t sz)
		: m_size(sz)
	{
	}

	~size_filter()
	{
		std::cout << __FUNCTION__ << std::endl;
	}

	bool handle_input(ranger::event::buffer&& src, ranger::event::buffer&& dst) final
	{
		size_t n = src.size() / m_size;
		if (n == 0)
		{
			return false;
		}

		std::vector<char> v(n * m_size);
		src.remove(&v.front(), v.size());
		dst.append(&v.front(), v.size());
		return true;
	}

	bool handle_output(ranger::event::buffer&& src, ranger::event::buffer&& dst) final
	{
		dst.append(src);
		return true;
	}

private:
	size_t m_size;
};

class transform_filter : public ranger::event::tcp_connection::filter_handler
{
private:
	~transform_filter()
	{
		std::cout << __FUNCTION__ << std::endl;
	}

	void transform(ranger::event::buffer& src, ranger::event::buffer& dst)
	{
		std::vector<char> v(src.size());
		src.remove(&v.front(), v.size());
		for (auto& ch: v)
			if ('a' <= ch && ch < 'z') ++ch;
		dst.append(&v.front(), v.size());
	}

	bool handle_input(ranger::event::buffer&& src, ranger::event::buffer&& dst) final
	{
		transform(src, dst);
		return true;
	}

	bool handle_output(ranger::event::buffer&& src, ranger::event::buffer&& dst) final
	{
		transform(src, dst);
		return true;
	}
};

class filter_server
	: public ranger::event::tcp_acceptor::event_handler
	, public ranger::event::tcp_connection::event_handler
{
public:
	filter_server(int port, size_t sz)
		: m_acc(ranger::event::tcp_acceptor::create(m_disp, ranger::event::endpoint(port)))
		, m_size(sz)
	{
		m_acc->set_event_handler(this);
	}

	int run()
	{
		return m_disp.run();
	}

private:
	void handle_accept(ranger::event::tcp_acceptor& acc, ranger::event::tcp_connection& conn) final
	{
		auto ep = conn.remote_endpoint();
		std::cout << "accept connection[" << ep.addr() << ":" << ep.port() << "]." << std::endl;

		conn.append_filter<size_filter>(m_size);
		conn.append_filter<transform_filter>();
		conn.set_event_handler(this);
		m_conn_map[&conn] = conn.shared_from_this();
	}

	void handle_read(ranger::event::tcp_connection& conn, ranger::event::buffer&& buf) final
	{
		std::vector<char> v(buf.size());
		buf.remove(&v.front(), v.size());
		for (auto ch: v) std::cout << ch << std::flush;
		conn.send(&v.front(), v.size());
	}

	void handle_timeout(ranger::event::tcp_connection& conn) final
	{
		auto ep = conn.remote_endpoint();
		std::cerr << "connection[" << ep.addr() << ":" << ep.port() << "] " << "timeout." << std::endl;

		m_conn_map.erase(&conn);
	}

	void handle_error(ranger::event::tcp_connection& conn) final
	{
		auto ep = conn.remote_endpoint();
		std::cerr << "connection[" << ep.addr() << ":" << ep.port() << "] " << "error[" << conn.error_code() << "]: " << conn.error_description() << std::endl;

		m_conn_map.erase(&conn);
	}

	void handle_eof(ranger::event::tcp_connection& conn) final
	{
		auto ep = conn.remote_endpoint();
		std::cerr << "connection[" << ep.addr() << ":" << ep.port() << "] " << "eof." << std::endl;

		m_conn_map.erase(&conn);
	}

private:
	ranger::event::dispatcher m_disp;
	std::shared_ptr<ranger::event::tcp_acceptor> m_acc;
	size_t m_size;

	std::map<ranger::event::tcp_connection*, std::shared_ptr<ranger::event::tcp_connection> > m_conn_map;
};

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: filter_server <port> <size>" << std::endl;
		return -1;
	}

	return filter_server(atoi(argv[1]), atoi(argv[2])).run();
}
