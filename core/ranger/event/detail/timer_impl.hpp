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

#ifndef RANGER_EVENT_DETAIL_TIMER_IMPL_HPP
#define RANGER_EVENT_DETAIL_TIMER_IMPL_HPP

#include <chrono>

struct event;

namespace ranger { namespace event {

class dispatcher;
class timer;

namespace detail {

class timer_impl {
public:
	explicit timer_impl(dispatcher& disp);
	~timer_impl();

	timer_impl(const timer_impl&) = delete;
	timer_impl& operator = (const timer_impl&) = delete;

	void set_handle(timer* hdl) {
		m_handle = hdl;
	}

	timer* get_handle() const {
		return m_handle;
	}

	template <class Rep, class Period>
	void active(const std::chrono::duration<Rep, Period>& dur) {
		auto sec = std::chrono::duration_cast<std::chrono::seconds>(dur);
		auto usec = std::chrono::duration_cast<std::chrono::microseconds>(dur - sec);
		active_impl(sec.count(), usec.count());
	}

private:
	void active_impl(long sec, long usec);

	timer* m_handle {nullptr};
	struct event* m_event;
};

}

} }

#endif	// RANGER_EVENT_DETAIL_TIMER_IMPL_HPP
