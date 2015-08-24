require "ranger_lua_event"
event = ranger_lua_event

tick_cnt = 10

disp = event.dispatcher()
sig = event.signal(disp, 15)
sig:set_event_handler(function(self)
	if tick_cnt > 0 then
		tick_cnt = tick_cnt - 1
		print("signal 15:", tick_cnt)
		sig:active()
	end
end)

sig:active()
disp:run()
