#include <event/dispatcher.hpp>
#include <event/timer.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		std::cerr << "Usage: timer_event <period> <tick_cnt>" << std::endl;
		return -1;
	}

	float period = atof(argv[1]);
	int tick_cnt = atoi(argv[2]);

	ranger::event::dispatcher disp;
	auto tmr = ranger::event::timer::create(disp, [period, &tick_cnt] (ranger::event::timer& tmr)
			{
				std::cout << "Tick." << std::endl;
				
				if (--tick_cnt > 0) tmr.active(period);
			});
	tmr->active(period);
	return disp.run();
}
