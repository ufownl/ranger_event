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

#include "signal.hpp"
#include "dispatcher.hpp"
#include <event2/event.h>
#include <stdexcept>

namespace ranger { namespace event {

	signal::~signal()
	{
		if (m_event)
		{
			event_free(m_event);
		}
	}

	std::shared_ptr<signal> signal::create(dispatcher& disp, int sig, const event_handler& handler)
	{
		return std::make_shared<signal>(disp, sig, handler);
	}

	std::shared_ptr<signal> signal::create(dispatcher& disp, int sig, event_handler&& handler)
	{
		return std::make_shared<signal>(disp, sig, std::move(handler));
	}

	void signal::active()
	{
		if (m_event)
		{
			event_add(m_event, nullptr);
		}
	}

	signal::signal(dispatcher& disp, int sig, const event_handler& handler)
		: m_event_handler(handler)
	{
		_init(disp._event_base(), sig);
	}

	signal::signal(dispatcher& disp, int sig, event_handler&& handler)
		: m_event_handler(std::move(handler))
	{
		_init(disp._event_base(), sig);
	}

	namespace
	{

		void handle_signal(evutil_socket_t fd, short what, void* ctx)
		{
			auto sig = static_cast<signal*>(ctx)->shared_from_this();
			auto& handler = sig->get_event_handler();
			if (handler)
			{
				handler(*sig);
			}
		}

	}

	void signal::_init(event_base* base, int sig)
	{
		m_event = event_new(base, sig, EV_SIGNAL, handle_signal, this);
		if (!m_event)
			throw std::runtime_error("event create failed.");
	}

} }
