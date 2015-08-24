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

#ifndef RANGER_EVENT_TCP_CONNECTION_HPP
#define RANGER_EVENT_TCP_CONNECTION_HPP

#ifndef SWIG
#include "ranger/event/endpoint.hpp"
#include "ranger/event/token_bucket_cfg.hpp"
#include <functional>

struct bufferevent;
struct event_base;
#endif	// !SWIG

namespace ranger { namespace event {

#ifndef SWIG
class buffer;
class dispatcher;
#endif	// !SWIG

class tcp_connection {
public:
	enum class event_code {
		read,
		write,
		connected,
		timeout,
		error,
		eof,
		invalid
	};

#ifndef SWIG
	using event_handler = std::function<void(tcp_connection&, event_code)>;

	struct filter_handler {
		virtual ~filter_handler() {}
		virtual bool handle_input(buffer&&, buffer&&) = 0;
		virtual bool handle_output(buffer&&, buffer&&) = 0;
	};
#endif	// !SWIG

	tcp_connection()
		: m_top_bev(nullptr)
		, m_base_bev(nullptr) {
		// nop
	}

	tcp_connection(dispatcher& disp, const endpoint& ep)
		: tcp_connection(disp) {
		connect(ep);
	}

	tcp_connection(dispatcher& disp, const char* addr, int port)
		: tcp_connection(disp) {
		connect(addr, port);
	}

#ifndef SWIG
	tcp_connection(dispatcher& disp, const std::string& addr, int port)
		: tcp_connection(disp, addr.c_str(), port) {
		// nop
	}

	template <class T>
	tcp_connection(dispatcher& disp, T&& handler, const endpoint& ep)
		: tcp_connection(disp) {
		m_event_handler = std::forward<T>(handler);
		connect(ep);
	}

	template <class T>
	tcp_connection(dispatcher& disp, T&& handler, const char* addr, int port)
		: tcp_connection(disp) {
		m_event_handler = std::forward<T>(handler);
		connect(addr, port);
	}

	template <class T>
	tcp_connection(dispatcher& disp, T&& handler, const std::string& addr, int port)
		: tcp_connection(disp, std::forward<T>(handler), addr.c_str(), port) {
	}
#endif	// !SWIG

	tcp_connection(dispatcher& disp, int fd);
	~tcp_connection();

#ifndef SWIG
	tcp_connection(const tcp_connection&) = delete;
	tcp_connection& operator = (const tcp_connection&) = delete;

	tcp_connection(tcp_connection&& rhs) noexcept
		: m_top_bev(rhs.m_top_bev)
		, m_base_bev(rhs.m_base_bev)
		, m_token_bucket(std::move(rhs.m_token_bucket))
		, m_event_handler(std::move(rhs.m_event_handler))
		, m_extra_data(rhs.m_extra_data) {
		reset_callbacks();
		rhs.m_top_bev = nullptr;
		rhs.m_base_bev = nullptr;
		rhs.m_extra_data = nullptr;
	}

	tcp_connection& operator = (tcp_connection&& rhs) noexcept {
		if (this != &rhs) {
			auto tmp = std::move(rhs);
			swap(tmp);
		}

		return *this;
	}
	
	static std::pair<tcp_connection, tcp_connection> create_pair(dispatcher& first_disp, dispatcher& second_disp);
	static std::pair<tcp_connection, tcp_connection> create_pair(dispatcher& disp);
#endif	// !SWIG
	static void file_descriptor_close(int fd);

	buffer read_buffer();
	buffer write_buffer();

#ifndef SWIG
	template <class ReadRep, class ReadPeriod, class WriteRep, class WritePeriod>
	void set_timeouts(	const std::chrono::duration<ReadRep, ReadPeriod>& read_timeout,
						const std::chrono::duration<WriteRep, WritePeriod>& write_timeout) {
		auto read_sec = std::chrono::duration_cast<std::chrono::seconds>(read_timeout);
		auto read_usec = std::chrono::duration_cast<std::chrono::microseconds>(read_timeout - read_sec);
		auto write_sec = std::chrono::duration_cast<std::chrono::seconds>(write_timeout);
		auto write_usec = std::chrono::duration_cast<std::chrono::microseconds>(write_timeout - write_sec);
		set_timeouts_impl(read_sec.count(), read_usec.count(), write_sec.count(), write_usec.count());
	}
#endif	// !SWIG

	void set_rate_limit(std::shared_ptr<const token_bucket_cfg> cfg);
	void reset_rate_limit();
	bool decrement_read_limit(ssize_t decr);
	bool decrement_write_limit(ssize_t decr);

	ssize_t get_read_limit() const;
	ssize_t get_write_limit() const;

	void set_nodelay(int val);
	int get_nodelay() const;

	int file_descriptor() const;
	endpoint remote_endpoint() const;

	static int error_code();
	static const char* error_description();

#ifndef SWIG
	template <class T, class... ARGS>
	void append_filter(ARGS&&... args) {
		if (m_top_bev) {
			append_filter_impl(std::unique_ptr<filter_handler>(new T(std::forward<ARGS>(args)...)));
		}
	}

	template <class T>
	void set_event_handler(T&& handler) {
		m_event_handler = std::forward<T>(handler);
	}

	const event_handler& get_event_handler() const {
		return m_event_handler;
	}
#endif	// !SWIG

	void set_extra_data(void* extra) {
		m_extra_data = extra;
	}

	void* get_extra_data() const {
		return m_extra_data;
	}

	void close() {
		tcp_connection(std::move(*this));
	}

#ifndef SWIG
	void swap(tcp_connection& rhs) noexcept {
		if (this != &rhs) {
			using std::swap;
			swap(m_top_bev, rhs.m_top_bev);
			swap(m_base_bev, rhs.m_base_bev);
			swap(m_token_bucket, rhs.m_token_bucket);
			swap(m_event_handler, rhs.m_event_handler);
			swap(m_extra_data, rhs.m_extra_data);

			reset_callbacks();
			rhs.reset_callbacks();
		}
	}
	
private:
	explicit tcp_connection(bufferevent* bev);
	explicit tcp_connection(dispatcher& disp);

	void connect(const endpoint& ep);
	void connect(const char* addr, int port);

	void set_timeouts_impl(long read_sec, long read_usec, long write_sec, long write_usec);
	void append_filter_impl(std::unique_ptr<filter_handler> filter);
	void reset_callbacks();

	bufferevent* m_top_bev;
	bufferevent* m_base_bev;
	std::shared_ptr<const token_bucket_cfg> m_token_bucket;
	event_handler m_event_handler;
	void* m_extra_data = nullptr;
#endif	// !SWIG
};

inline void swap(tcp_connection& lhs, tcp_connection& rhs) noexcept {
	lhs.swap(rhs);
}

} }

#ifndef SWIG
namespace std {

template <>
struct less<ranger::event::tcp_connection> {
	using result_type = bool;
	using first_argument_type = ranger::event::tcp_connection;
	using second_argument_type = ranger::event::tcp_connection;

	result_type operator () (const first_argument_type& lhs, const second_argument_type& rhs) const {
		return lhs.file_descriptor() < rhs.file_descriptor();
	}
};

template <>
struct equal_to<ranger::event::tcp_connection> {
	using result_type = bool;
	using first_argument_type = ranger::event::tcp_connection;
	using second_argument_type = ranger::event::tcp_connection;

	result_type operator () (const first_argument_type& lhs, const second_argument_type& rhs) const {
		return lhs.file_descriptor() == rhs.file_descriptor();
	}
};

template <>
struct hash<ranger::event::tcp_connection> {
	using result_type = size_t;
	using argument_type = ranger::event::tcp_connection;

	result_type operator () (const argument_type& conn) const {
		return conn.file_descriptor();
	}
};

}
#endif	// !SWIG

#endif	// RANGER_EVENT_TCP_CONNECTION_HPP
