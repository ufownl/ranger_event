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

#include "ranger/event/dispatcher.hpp"
#include <event2/event.h>
#include <event2/thread.h>
#include <mutex>
#include <stdexcept>

namespace ranger { namespace event {

dispatcher::dispatcher() {
	static std::once_flag flag;
	std::call_once(flag, [] {
		if (evthread_use_pthreads() == -1)
		throw std::runtime_error("evthread_use_pthreads call failed.");
	});

	m_base = event_base_new();
	if (!m_base)
		throw std::runtime_error("event_base create faild.");
}

dispatcher::~dispatcher() {
	if (m_base)
		event_base_free(m_base);
}

int dispatcher::run() {
	return m_base ? event_base_dispatch(m_base) : 1;
}

int dispatcher::run_once(bool is_block /* = true */) {
	return m_base ? event_base_loop(m_base, is_block ? EVLOOP_ONCE : EVLOOP_ONCE | EVLOOP_NONBLOCK) : 1;
}

void dispatcher::kill() {
	event_base_loopbreak(m_base);
}

void dispatcher::_exit(long sec, long usec) {
	if (sec > 0 || usec > 0) {
		timeval tv;
		tv.tv_sec = sec;
		tv.tv_usec = usec;
		event_base_loopexit(m_base, &tv);
	} else
		event_base_loopexit(m_base, nullptr);
}

} }
