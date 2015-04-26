#include <event/dispatcher.hpp>
#include <event/tcp_connection.hpp>
#include <iostream>

int main()
{
	ranger::event::dispatcher disp;
	ranger::event::tcp_connection conn(disp, "www.baidu.com", 80);
	std::cout << "nodelay: " << conn.get_nodelay() << std::endl;
	conn.set_nodelay(1);
	std::cout << conn.error_description() << std::endl;
	std::cout << "nodelay: " << conn.get_nodelay() << std::endl;;
	conn.set_nodelay(0);
	std::cout << conn.error_description() << std::endl;
	std::cout << "nodelay: " << conn.get_nodelay() << std::endl;
	return 0;
}
