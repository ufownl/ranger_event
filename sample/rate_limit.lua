require "ranger_lua_event"
event = ranger_lua_event

disp = event.dispatcher()
conn = event.tcp_connection(disp, "www.baidu.com", 80)
conn:set_rate_limit(event.create_token_bucket_cfg(10, 50, 1000, 1000, 0.2))
conn:set_event_handler(function(conn, what)
	if what == event.tcp_connection_event_code_read then
		local buf = conn:read_buffer()
		local size = buf:size()
		local bytes = event.byte_array(size)
		buf:remove(bytes, size)
		local str = ""
		for i = 0, size - 1 do
			str = str..string.format("%c", bytes[i])
		end
		io.write(str)
		io.flush()
	elseif what == event.tcp_connection_event_code_connected then
		local ep = conn:remote_endpoint()
		print("connection["..ep:addr()..":"..ep:port().."] connected.")
	elseif what == event.tcp_connection_event_code_error then
		local ep = conn:remote_endpoint()
		print("connection["..ep:addr()..":"..ep:port().."] error: "..event.tcp_connection.error_description());
	end
end)

conn:write_buffer():append(event.str2cdata("GET / HTTP/1.1\r\n\r\n"))
disp:run()
