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
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <errno.h>
#include <stdexcept>

namespace ranger { namespace event {

	tcp_connection::~tcp_connection()
	{
		if (m_top_bev)
		{
			bufferevent_free(m_top_bev);
		}
	}

	std::shared_ptr<tcp_connection> tcp_connection::create(dispatcher& disp, const endpoint& ep)
	{
		return std::make_shared<tcp_connection>(disp, ep);
	}

	std::shared_ptr<tcp_connection> tcp_connection::create(dispatcher& disp, const char* addr, int port)
	{
		return std::make_shared<tcp_connection>(disp, addr, port);
	}

	std::shared_ptr<tcp_connection> tcp_connection::create(dispatcher& disp, const std::string& addr, int port)
	{
		return std::make_shared<tcp_connection>(disp, addr, port);
	}

	bool tcp_connection::send(const void* src, size_t len)
	{
		if (!m_top_bev)
		{
			return false;
		}

		return bufferevent_write(m_top_bev, src, len) == 0;
	}

	bool tcp_connection::send(buffer& src)
	{
		if (!m_top_bev)
		{
			return false;
		}

		return bufferevent_write_buffer(m_top_bev, src._evbuffer()) == 0;
	}

	void tcp_connection::set_rate_limit(const token_bucket_cfg& cfg)
	{
		if (m_base_bev)
		{
			bufferevent_set_rate_limit(m_base_bev, cfg._ev_token_bucket_cfg());
			m_token_bucket = cfg.shared_from_this();
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
		{
			return false;
		}

		return bufferevent_decrement_read_limit(m_base_bev, decr);
	}

	bool tcp_connection::decrement_write_limit(ssize_t decr)
	{
		if (!m_base_bev)
		{
			return false;
		}

		return bufferevent_decrement_write_limit(m_base_bev, decr);
	}

	ssize_t tcp_connection::get_read_limit() const
	{
		if (!m_base_bev)
		{
			return 0;
		}

		return bufferevent_get_read_limit(m_base_bev);
	}

	ssize_t tcp_connection::get_write_limit() const
	{
		if (!m_base_bev)
		{
			return 0;
		}

		return bufferevent_get_write_limit(m_base_bev);
	}

	endpoint tcp_connection::remote_endpoint() const
	{
		if (!m_base_bev)
		{
			return endpoint();
		}

		evutil_socket_t fd = bufferevent_getfd(m_base_bev);
		if (fd == -1)
		{
			return endpoint();
		}

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

	tcp_connection::tcp_connection(dispatcher& disp, const endpoint& ep)
		: tcp_connection(bufferevent_socket_new(disp._event_base(), -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE | BEV_OPT_UNLOCK_CALLBACKS | BEV_OPT_DEFER_CALLBACKS))
	{
		if (bufferevent_socket_connect(m_base_bev, (sockaddr*)&ep._sockaddr_in(), sizeof(sockaddr_in)) == -1)
		{
			bufferevent_free(m_top_bev);
			throw std::runtime_error("bufferevent_socket_connect call failed.");
		}
	}

	tcp_connection::tcp_connection(dispatcher& disp, const char* addr, int port)
		: tcp_connection(bufferevent_socket_new(disp._event_base(), -1, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE | BEV_OPT_UNLOCK_CALLBACKS | BEV_OPT_DEFER_CALLBACKS))
	{
		if (bufferevent_socket_connect_hostname(m_base_bev, 0, AF_UNSPEC, addr, port) == -1)
		{
			bufferevent_free(m_top_bev);
			throw std::runtime_error("bufferevent_socket_connect_hostname call failed.");
		}
	}

	tcp_connection::tcp_connection(dispatcher& disp, const std::string& addr, int port)
		: tcp_connection(disp, addr.c_str(), port)
	{
	}

	tcp_connection::tcp_connection(event_base* base, int fd)
		: tcp_connection(bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE | BEV_OPT_THREADSAFE | BEV_OPT_UNLOCK_CALLBACKS | BEV_OPT_DEFER_CALLBACKS))
	{
	}

	namespace
	{

		void handle_read(bufferevent* bev, void* ctx)
		{
			auto conn = static_cast<tcp_connection*>(ctx)->shared_from_this();
			auto handler = conn->get_event_handler();
			if (handler)
			{
				handler->handle_read(*conn, buffer(bufferevent_get_input(bev)));
			}
		}

		void handle_write(bufferevent* bev, void* ctx)
		{
			auto conn = static_cast<tcp_connection*>(ctx)->shared_from_this();
			auto handler = conn->get_event_handler();
			if (handler)
			{
				handler->handle_write(*conn, buffer(bufferevent_get_output(bev)));
			}
		}

		void handle_event(bufferevent* bev, short what, void* ctx)
		{
			auto conn = static_cast<tcp_connection*>(ctx)->shared_from_this();
			auto handler = conn->get_event_handler();
			if (handler)
			{
				if (what & BEV_EVENT_EOF) handler->handle_eof(*conn);
				else if (what & BEV_EVENT_ERROR) handler->handle_error(*conn);
				else if (what & BEV_EVENT_TIMEOUT) handler->handle_timeout(*conn);
				else if (what & BEV_EVENT_CONNECTED) handler->handle_connected(*conn);
			}
		}

	}

	tcp_connection::tcp_connection(bufferevent* bev)
		: m_top_bev(bev)
		, m_base_bev(bev)
	{
		if (!bev)
		{
			throw std::runtime_error("bufferevent create failed.");
		}

		bufferevent_setcb(bev, handle_read, handle_write, handle_event, this);
		bufferevent_enable(bev, EV_READ | EV_WRITE);
	}

} }
