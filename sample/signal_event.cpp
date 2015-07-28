#include <event/dispatcher.hpp>
#include <event/signal.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cerr << "Usage: signal_event <sig_num> <tick_cnt>" << std::endl;
		return -1;
	}

	int sig_num = atoi(argv[1]);
	size_t tick_cnt = atoi(argv[2]);

	ranger::event::dispatcher disp;
	ranger::event::signal sig(disp, sig_num, [sig_num, &tick_cnt] (ranger::event::signal& sig) {
		if (tick_cnt-- > 0) {
			std::cout << "signal: " << sig_num << " tick: " << tick_cnt << std::endl;
			sig.active();
		}
	});

	sig.active();

	return disp.run();
}
