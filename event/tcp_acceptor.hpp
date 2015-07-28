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

#ifndef RANGER_EVENT_TCP_ACCEPTOR_HPP
#define RANGER_EVENT_TCP_ACCEPTOR_HPP

#include <functional>

struct evconnlistener;

namespace ranger { namespace event {

class dispatcher;
class endpoint;
class tcp_connection;

class tcp_acceptor {
public:
	using event_handler = std::function<bool(tcp_acceptor&, int)>;

public:
	tcp_acceptor(dispatcher& disp, const endpoint& ep, int backlog = -1) {
		_bind(disp, ep, backlog);
	}

	template <class T>
	tcp_acceptor(dispatcher& disp, T&& handler, const endpoint& ep, int backlog = -1) {
		m_event_handler = std::forward<T>(handler);
		_bind(disp, ep, backlog);
	}

	~tcp_acceptor();

	tcp_acceptor(const tcp_acceptor&) = delete;
	tcp_acceptor& operator = (const tcp_acceptor&) = delete;

	tcp_acceptor(tcp_acceptor&& rhs)
		: m_listener(rhs.m_listener)
		, m_event_handler(std::move(rhs.m_event_handler)) {
		_reset_callbacks();
		rhs.m_listener = nullptr;
	}

	tcp_acceptor& operator = (tcp_acceptor&& rhs) {
		if (this != &rhs) {
			tcp_acceptor acc = std::move(rhs);
			swap(acc);
		}

		return *this;
	}

	int file_descriptor() const;
	endpoint local_endpoint() const;

	template <class T>
	void set_event_handler(T&& handler) {
		m_event_handler = std::forward<T>(handler);
	}

	const event_handler& get_event_handler() const {
		return m_event_handler;
	}

	void set_extra_data(void* extra) {
		m_extra_data = extra;
	}

	void* get_extra_data() const {
		return m_extra_data;
	}

	void close() {
		tcp_acceptor(std::move(*this));
	}

	void swap(tcp_acceptor& rhs) {
		using std::swap;
		swap(m_listener, rhs.m_listener);
		swap(m_event_handler, rhs.m_event_handler);

		_reset_callbacks();
		rhs._reset_callbacks();
	}

private:
	void _bind(dispatcher& disp, const endpoint& ep, int backlog);

	void _reset_callbacks();

private:
	evconnlistener* m_listener;
	event_handler m_event_handler;
	void* m_extra_data = nullptr;
};

inline void swap(tcp_acceptor& lhs, tcp_acceptor& rhs) {
	lhs.swap(rhs);
}

} }

namespace std {

template <>
struct less<ranger::event::tcp_acceptor> {
	using result_type = bool;
	using first_argument_type = ranger::event::tcp_acceptor;
	using second_argument_type = ranger::event::tcp_acceptor;

	result_type operator () (const first_argument_type& lhs, const second_argument_type& rhs) const {
		return lhs.file_descriptor() < rhs.file_descriptor();
	}
};

template <>
struct equal_to<ranger::event::tcp_acceptor> {
	using result_type = bool;
	using first_argument_type = ranger::event::tcp_acceptor;
	using second_argument_type = ranger::event::tcp_acceptor;

	result_type operator () (const first_argument_type& lhs, const second_argument_type& rhs) const {
		return lhs.file_descriptor() == rhs.file_descriptor();
	}
};

template <>
struct hash<ranger::event::tcp_acceptor> {
	using result_type = size_t;
	using argument_type = ranger::event::tcp_acceptor;

	result_type operator () (const argument_type& acc) const {
		return acc.file_descriptor();
	}
};

}

#endif	// RANGER_EVENT_TCP_ACCEPTOR_HPP
