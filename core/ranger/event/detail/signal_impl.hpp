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

#ifndef RANGER_EVENT_DETAIL_SIGNAL_IMPL_HPP
#define RANGER_EVENT_DETAIL_SIGNAL_IMPL_HPP

struct event;

namespace ranger { namespace event {

class dispatcher;
class signal;

namespace detail {

class signal_impl {
public:
	signal_impl(dispatcher& disp, int sig);
	~signal_impl();

	signal_impl(const signal_impl&) = delete;
	signal_impl& operator = (const signal_impl&) = delete;

	void set_handle(signal* hdl) {
		m_handle = hdl;
	}

	signal* get_handle() const {
		return m_handle;
	}

	void active();

private:
	signal* m_handle {nullptr};
	struct event* m_event;
};

}

} }

#endif	// RANGER_EVENT_DETAIL_SIGNAL_IMPL_HPP
