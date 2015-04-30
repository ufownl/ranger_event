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
	ranger::event::timer tmr(disp, [period, &tick_cnt] (ranger::event::timer& tmr)
			{
				if (tick_cnt-- > 0)
				{
					std::cout << "tick: " << tick_cnt << std::endl;
					tmr.active(std::chrono::duration<float>(period));
				}
			});
	tmr.active(std::chrono::duration<float>(period));

	return disp.run();
}
