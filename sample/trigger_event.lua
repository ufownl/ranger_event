require "ranger_lua_event"
event = ranger_lua_event

tick_cnt = 10

disp = event.dispatcher()
tr = event.trigger(disp)
tr:set_event_handler(function(self)
	if tick_cnt > 0 then
		tick_cnt = tick_cnt - 1
		print("touch:", tick_cnt)
		tr:active()
	end
end)

tr:active()
disp:run()
