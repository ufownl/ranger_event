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

#include "tcp_acceptor.hpp"
#include "dispatcher.hpp"
#include "endpoint.hpp"
#include "tcp_connection.hpp"
#include "../util/scope_guard.hpp"
#include <event2/listener.h>
#include <stdexcept>

namespace ranger { namespace event {

	namespace
	{

		void handle_accept(evconnlistener* listener, evutil_socket_t fd, sockaddr* addr, int socklen, void* ctx)
		{
			util::scope_guard fd_guard([fd] { evutil_closesocket(fd); });

			auto acc = static_cast<tcp_acceptor*>(ctx);
			auto& handler = acc->get_event_handler();
			if (handler && handler(*acc, fd))
				fd_guard.dismiss();
		}

	}

	tcp_acceptor::~tcp_acceptor()
	{
		if (m_listener)
			evconnlistener_free(m_listener);
	}

	int tcp_acceptor::file_descriptor() const
	{
		if (!m_listener)
			return -1;

		return evconnlistener_get_fd(m_listener);
	}

	endpoint tcp_acceptor::local_endpoint() const
	{
		evutil_socket_t fd = file_descriptor();
		if (fd == -1)
			return endpoint();

		sockaddr_in sin;
		socklen_t len = sizeof(sin);
		getsockname(fd, (sockaddr*)&sin, &len);
		return endpoint(sin);
	}

	void tcp_acceptor::_bind(dispatcher& disp, const endpoint& ep, int backlog)
	{
		m_listener = evconnlistener_new_bind(disp._event_base(), handle_accept, this, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, backlog, (sockaddr*)&ep._sockaddr_in(), sizeof(sockaddr_in));
		if (!m_listener)
			throw std::runtime_error("evconnlistener create failed.");
	}

	void tcp_acceptor::_reset_callbacks()
	{
		if (m_listener)
			evconnlistener_set_cb(m_listener, handle_accept, this);
	}

} }
