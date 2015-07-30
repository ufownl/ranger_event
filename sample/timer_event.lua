require "lua_ranger_event"
event = lua_ranger_event

tick_cnt = 10

disp = event.dispatcher()
tmr = event.timer(disp)
tmr:set_event_handler(function(self)
	if tick_cnt > 0 then
		tick_cnt = tick_cnt - 1
		print("tick:", tick_cnt)
		tmr:active(1)
	end
end)

tmr:active(1)
disp:run()
