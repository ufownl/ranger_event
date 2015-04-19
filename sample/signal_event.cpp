#include <event/dispatcher.hpp>
#include <event/signal.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: signal_event <sig_num> <tick_cnt>" << std::endl;
		return -1;
	}

	int sig_num = atoi(argv[1]);
	size_t tick_cnt = atoi(argv[2]);

	ranger::event::dispatcher disp;
	auto sig = ranger::event::signal::create(disp, sig_num, [sig_num, &tick_cnt] (ranger::event::signal& sig)
			{
				std::cout << "signal: " << sig_num << std::endl;
				if (--tick_cnt > 0) sig.active();
			});
	sig->active();
	return disp.run();
}
