#include <event/dispatcher.hpp>
#include <event/trigger.hpp>
#include <iostream>

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "Usage: trigger_event <tick_cnt>" << std::endl;
		return -1;
	}

	size_t tick_cnt = atoi(argv[1]);

	ranger::event::dispatcher disp;
	ranger::event::trigger tr(disp, [&tick_cnt] (ranger::event::trigger& tr)
			{
				if (tick_cnt-- > 0)
				{
					std::cout << "touch: " << tick_cnt << std::endl;
					tr.active();
				}
			});
	tr.active();

	return disp.run();
}
