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
#include <event2/listener.h>
#include <stdexcept>

namespace ranger { namespace event {

	tcp_acceptor::~tcp_acceptor()
	{
		if (m_listener)
			evconnlistener_free(m_listener);
	}

	std::shared_ptr<tcp_acceptor> tcp_acceptor::create(dispatcher& disp, const endpoint& ep, int backlog /* = -1 */)
	{
		return std::make_shared<tcp_acceptor>(disp, ep, backlog);
	}

	namespace
	{

		void handle_accept(evconnlistener* listener, evutil_socket_t fd, sockaddr* addr, int socklen, void* ctx)
		{
			auto acc = static_cast<tcp_acceptor*>(ctx)->shared_from_this();
			auto conn = std::make_shared<tcp_connection>(evconnlistener_get_base(listener), fd);
			auto handler = acc->get_event_handler();
			if (handler)
				handler->handle_accept(*acc, *conn);
		}

	}

	tcp_acceptor::tcp_acceptor(dispatcher& disp, const endpoint& ep, int backlog)
	{
		//m_listener = evconnlistener_new_bind(disp._event_base(), handle_accept, this, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE | LEV_OPT_THREADSAFE, backlog, (sockaddr*)&ep._sockaddr_in(), sizeof(sockaddr_in));
		m_listener = evconnlistener_new_bind(disp._event_base(), handle_accept, this, LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, backlog, (sockaddr*)&ep._sockaddr_in(), sizeof(sockaddr_in));
		if (!m_listener)
			throw std::runtime_error("evconnlistener create failed.");
	}

} }
