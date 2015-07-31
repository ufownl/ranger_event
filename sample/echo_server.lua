require "lua_ranger_event"
event = lua_ranger_event

disp = event.dispatcher()
acc = event.tcp_acceptor(disp, event.endpoint(50000))
conn_table = {}
acc:set_event_handler(function(acc, fd)
	local conn = event.tcp_connection(disp, fd)
	conn:set_event_handler(function(conn, what)
		if what == event.tcp_connection_event_code_read then
			conn:write_buffer():append(conn:read_buffer())
		elseif what == event.tcp_connection_event_code_timeout then
			local ep = conn:remote_endpoint()
			print("connection["..ep:addr()..":"..ep:port().."] timeout.")
			conn_table[conn:file_descriptor()] = nil
			conn:close();
		elseif what == event.tcp_connection_event_code_error then
			local ep = conn:remote_endpoint()
			print("connection["..ep:addr()..":"..ep:port().."] error: "..event.tcp_connection.error_description());
			conn_table[conn:file_descriptor()] = nil
			conn:close();
		elseif what == event.tcp_connection_event_code_eof then
			local ep = conn:remote_endpoint()
			print("connection["..ep:addr()..":"..ep:port().."] eof.")
			conn_table[conn:file_descriptor()] = nil
			conn:close();
		end
	end)

	local local_ep = acc:local_endpoint()
	local remote_ep = conn:remote_endpoint()
	print("acceptor["..local_ep:addr()..":"..local_ep:port().."] accept connection["..remote_ep:addr()..":"..remote_ep:port().."].")

	conn_table[conn:file_descriptor()] = conn

	return true
end)

disp:run()
