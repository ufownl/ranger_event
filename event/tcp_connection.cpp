// Copyright (c) 2015, RangerUFO
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
// 
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// 
// * Neither the name of ranger_event nor the names of its
//   contributors may be used to endorse or promote products derived from
//   this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "tcp_connection.hpp"
#include "dispatcher.hpp"
#include "endpoint.hpp"
#include "buffer.hpp"
#include "../util/scope_guard.hpp"
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <string.h>
#include <stdexcept>

namespace ranger { namespace event {

	tcp_connection::tcp_connection(dispatcher& disp, int fd)
		: tcp_connection(bufferevent_socket_new(disp._event_base(), fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS))
	{
	}

	tcp_connection::~tcp_connection()
	{
		if (m_top_bev)
			bufferevent_free(m_top_bev);
	}

	std::pair<tcp_connection, tcp_connection> tcp_connection::create_pair(dispatcher& first_disp, dispatcher& second_disp)
	{
		evutil_socket_t fd_pair[2];
		if (evutil_socketpair(AF_LOCAL, SOCK_STREAM, 0, fd_pair) == -1)
		{
			throw std::runtime_error("evutil_socketpair call failed.");
		}

		util::scope_guard fd_first_guard([fd_pair] { evutil_closesocket(fd_pair[0]); });
		util::scope_guard fd_second_guard([fd_pair] { evutil_closesocket(fd_pair[1]); });

		for (auto fd: fd_pair)
		{
			if (evutil_make_socket_nonblocking(fd) == -1)
				throw std::runtime_error("evutil_make_socket_nonblocking call failed.");
		}

		ranger::event::tcp_connection first_conn(first_disp, fd_pair[0]);
		fd_first_guard.dismiss();

		ranger::event::tcp_connection second_conn(second_disp, fd_pair[1]);
		fd_second_guard.dismiss();

		return std::make_pair(std::move(first_conn), std::move(second_conn));
	}

	std::pair<tcp_connection, tcp_connection> tcp_connection::create_pair(dispatcher& disp)
	{
		return create_pair(disp, disp);
	}

	void tcp_connection::file_descriptor_close(int fd)
	{
		evutil_closesocket(fd);
	}

	buffer tcp_connection::read_buffer()
	{
		return m_top_bev ? buffer(bufferevent_get_input(m_top_bev)) : buffer(nullptr);
	}

	buffer tcp_connection::write_buffer()
	{
		return m_top_bev ? buffer(bufferevent_get_output(m_top_bev)) : buffer(nullptr);
	}

	void tcp_connection::set_rate_limit(std::shared_ptr<const token_bucket_cfg> cfg)
	{
		if (m_base_bev)
		{
			bufferevent_set_rate_limit(m_base_bev, cfg->_ev_token_bucket_cfg());
			m_token_bucket = std::move(cfg);
		}
	}

	void tcp_connection::reset_rate_limit()
	{
		if (m_base_bev)
		{
			bufferevent_set_rate_limit(m_base_bev, nullptr);
			m_token_bucket.reset();
		}
	}

	bool tcp_connection::decrement_read_limit(ssize_t decr)
	{
		if (!m_base_bev)
			return false;

		return bufferevent_decrement_read_limit(m_base_bev, decr);
	}

	bool tcp_connection::decrement_write_limit(ssize_t decr)
	{
		if (!m_base_bev)
			return false;

		return bufferevent_decrement_write_limit(m_base_bev, decr);
	}

	ssize_t tcp_connection::get_read_limit() const
	{
		if (!m_base_bev)
			return 0;

		return bufferevent_get_read_limit(m_base_bev);
	}

	ssize_t tcp_connection::get_write_limit() const
	{
		if (!m_base_bev)
			return 0;

		return bufferevent_get_write_limit(m_base_bev);
	}

	void tcp_connection::set_nodelay(int val)
	{
		if (!m_base_bev)
			return;

		evutil_socket_t fd = bufferevent_getfd(m_base_bev);
		if (fd == -1)
			return;

		setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&val, sizeof(val));
	}

	int tcp_connection::get_nodelay() const
	{
		if (!m_base_bev)
			return false;

		evutil_socket_t fd = bufferevent_getfd(m_base_bev);
		if (fd == -1)
			return false;

		int val = 0;
		socklen_t len = sizeof(val);
		getsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char*)&val, &len);

		return val;
	}

	int tcp_connection::file_descriptor() const
	{
		if (!m_base_bev)
			return -1;

		return bufferevent_getfd(m_base_bev);
	}

	endpoint tcp_connection::remote_endpoint() const
	{
		evutil_socket_t fd = file_descriptor();
		if (fd == -1)
			return endpoint();

		sockaddr_in sin;
		socklen_t len = sizeof(sin);
		getpeername(fd, (sockaddr*)&sin, &len);
		return endpoint(sin);
	}

	int tcp_connection::error_code()
	{
		return EVUTIL_SOCKET_ERROR();
	}

	const char* tcp_connection::error_description()
	{
		return evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR());
	}

	tcp_connection::tcp_connection(bufferevent* bev)
		: m_top_bev(bev)
		, m_base_bev(bev)
	{
		if (!bev)
			throw std::runtime_error("bufferevent create failed.");

		_reset_callbacks();
	}

	tcp_connection::tcp_connection(dispatcher& disp)
		: tcp_connection(bufferevent_socket_new(disp._event_base(), -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS))
	{
	}

	void tcp_connection::_connect(const endpoint& ep)
	{
		if (bufferevent_socket_connect(m_base_bev, (sockaddr*)&ep._sockaddr_in(), sizeof(sockaddr_in)) == -1)
		{
			bufferevent_free(m_top_bev);
			throw std::runtime_error("bufferevent_socket_connect call failed.");
		}
	}

	void tcp_connection::_connect(const char* addr, int port)
	{
		if (bufferevent_socket_connect_hostname(m_base_bev, 0, AF_UNSPEC, addr, port) == -1)
		{
			bufferevent_free(m_top_bev);
			throw std::runtime_error("bufferevent_socket_connect_hostname call failed.");
		}
	}

	void tcp_connection::_set_timeouts(long read_sec, long read_usec, long write_sec, long write_usec)
	{
		if (m_base_bev)
		{
			timeval read_tv;
			timeval write_tv;

			if (read_sec > 0 || read_usec > 0)
			{
				read_tv.tv_sec = read_sec;
				read_tv.tv_usec = read_usec;
			}

			if (write_sec > 0 || write_usec > 0)
			{
				write_tv.tv_sec = write_sec;
				write_tv.tv_usec = write_usec;
			}

			bufferevent_set_timeouts(m_base_bev,
					read_sec > 0 || read_usec > 0 ? &read_tv : nullptr,
					write_sec > 0 || write_usec > 0 ? &write_tv : nullptr);
		}
	}

	namespace
	{

		bufferevent_filter_result handle_filter_input(evbuffer* src, evbuffer* dst, ev_ssize_t limit, bufferevent_flush_mode mode, void *ctx)
		{
			return static_cast<tcp_connection::filter_handler*>(ctx)->handle_input(buffer(src), buffer(dst)) ? BEV_OK : BEV_NEED_MORE;
		}

		bufferevent_filter_result handle_filter_output(evbuffer* src, evbuffer* dst, ev_ssize_t limit, bufferevent_flush_mode mode, void *ctx)
		{
			return static_cast<tcp_connection::filter_handler*>(ctx)->handle_output(buffer(src), buffer(dst)) ? BEV_OK : BEV_NEED_MORE;
		}

		void filter_free(void* filter)
		{
			delete static_cast<tcp_connection::filter_handler*>(filter);
		}

	}

	void tcp_connection::_append_filter(std::unique_ptr<filter_handler> filter)
	{
		std::unique_ptr<bufferevent, decltype(&bufferevent_free)> bev_filter(bufferevent_filter_new(m_top_bev, handle_filter_input, handle_filter_output, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS, filter_free, filter.get()), bufferevent_free);
		if (!bev_filter)
			throw std::runtime_error("bufferevent_filter create failed.");

		filter.release();
		m_top_bev = bev_filter.release();

		_reset_callbacks();
	}

	namespace
	{

		void handle_read(bufferevent* bev, void* ctx)
		{
			auto conn = static_cast<tcp_connection*>(ctx);
			auto& handler = conn->get_event_handler();
			if (handler)
				handler(*conn, tcp_connection::event_code::read);
		}

		void handle_write(bufferevent* bev, void* ctx)
		{
			auto conn = static_cast<tcp_connection*>(ctx);
			auto& handler = conn->get_event_handler();
			if (handler)
				handler(*conn, tcp_connection::event_code::write);
		}

		void handle_event(bufferevent* bev, short what, void* ctx)
		{
			auto conn = static_cast<tcp_connection*>(ctx);
			auto& handler = conn->get_event_handler();
			if (handler)
			{
				if (what & BEV_EVENT_EOF) handler(*conn, tcp_connection::event_code::eof);
				else if (what & BEV_EVENT_ERROR) handler(*conn, tcp_connection::event_code::error);
				else if (what & BEV_EVENT_TIMEOUT) handler(*conn, tcp_connection::event_code::timeout);
				else if (what & BEV_EVENT_CONNECTED) handler(*conn, tcp_connection::event_code::connected);
			}
		}

	}

	void tcp_connection::_reset_callbacks()
	{
		if (m_top_bev)
		{
			bufferevent_setcb(m_top_bev, handle_read, handle_write, handle_event, this);
			bufferevent_enable(m_top_bev, EV_READ | EV_WRITE);
		}
	}

} }
