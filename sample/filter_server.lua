require "lua_ranger_event"
event = lua_ranger_event

function make_size_filter(size)
	return function(src, dst)
		local n = math.floor(src:size() / size)
		if n == 0 then
			return false
		end

		local buf_size = n * size
		local buf = event.byte_array(buf_size)
		src:remove(buf, buf_size)
		dst:append(buf, buf_size)
		return true
	end, function(src, dst)
		dst:append(src)
		return true
	end
end

function make_transform_filter(offset)
	local function transform(src, dst)
		local buf_size = src:size()
		local buf = event.byte_array(src:size())
		src:remove(buf, buf_size)
		for i = 0, buf_size - 1 do
			if string.byte("a") <= buf[i] and buf[i] <= string.byte("z") then
				buf[i] = buf[i] + offset
			end
		end
		dst:append(buf, buf_size)
	end

	return function(src, dst)
		transform(src, dst)
		return true
	end, function(src, dst)
		transform(src, dst)
		return true
	end
end

disp = event.dispatcher()
acc = event.tcp_acceptor(disp, event.endpoint(50000))
conn_table = {}
acc:set_event_handler(function(acc, fd)
	local conn = event.tcp_connection(disp, fd)
	conn:append_filter(make_size_filter(10))
	conn:append_filter(make_transform_filter(1))
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
			conn:write_buffer():append(bytes, size)
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
