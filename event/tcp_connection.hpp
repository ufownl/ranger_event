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

#ifdef RANGER_EVENT_INTERNAL
#include "endpoint.hpp"
#include "token_bucket_cfg.hpp"
#else
#include <event/endpoint.hpp>
#include <event/token_bucket_cfg.hpp>
#endif	// RANGER_EVENT_INTERNAL
#include <functional>

struct bufferevent;
struct event_base;

namespace ranger { namespace event {

	class buffer;
	class dispatcher;

	class tcp_connection
	{
	public:
		enum class event_code
		{
			read,
			write,
			connected,
			timeout,
			error,
			eof,
			invalid
		};

		using event_handler = std::function<void(tcp_connection&, event_code)>;

		struct filter_handler
		{
			virtual ~filter_handler() {}
			virtual bool handle_input(buffer&&, buffer&&) = 0;
			virtual bool handle_output(buffer&&, buffer&&) = 0;
		};

	public:
		tcp_connection() : m_top_bev(nullptr), m_base_bev(nullptr) {}
		tcp_connection(dispatcher& disp, const endpoint& ep);
		tcp_connection(dispatcher& disp, const char* addr, int port);
		tcp_connection(dispatcher& disp, const std::string& addr, int port);
		tcp_connection(dispatcher& disp, int fd);
		~tcp_connection();

		tcp_connection(const tcp_connection&) = delete;
		tcp_connection& operator = (const tcp_connection&) = delete;

		tcp_connection(tcp_connection&& rhs)
			: m_top_bev(rhs.m_top_bev)
			, m_base_bev(rhs.m_base_bev)
			, m_token_bucket(std::move(rhs.m_token_bucket))
			, m_event_handler(std::move(rhs.m_event_handler))
			, m_extra_data(rhs.m_extra_data)
		{
			_reset_callbacks();

			rhs.m_top_bev = nullptr;
			rhs.m_base_bev = nullptr;
			rhs.m_extra_data = nullptr;
		}

		tcp_connection& operator = (tcp_connection&& rhs)
		{
			if (this != &rhs)
			{
				tcp_connection conn = std::move(rhs);
				swap(conn);
			}

			return *this;
		}
		
		static std::pair<tcp_connection, tcp_connection> create_pair(dispatcher& first_disp, dispatcher& second_disp);
		static std::pair<tcp_connection, tcp_connection> create_pair(dispatcher& disp);
		static void file_descriptor_close(int fd);

		buffer read_buffer();
		buffer write_buffer();

		template <class _read_rep, class _read_period, class _write_rep, class _write_period>
		void set_timeouts(const std::chrono::duration<_read_rep, _read_period>& read_timeout, const std::chrono::duration<_write_rep, _write_period>& write_timeout)
		{
			auto read_sec = std::chrono::duration_cast<std::chrono::seconds>(read_timeout);
			auto read_usec = std::chrono::duration_cast<std::chrono::microseconds>(read_timeout - read_sec);
			auto write_sec = std::chrono::duration_cast<std::chrono::seconds>(write_timeout);
			auto write_usec = std::chrono::duration_cast<std::chrono::microseconds>(write_timeout - write_sec);
			_set_timeouts(read_sec.count(), read_usec.count(), write_sec.count(), write_usec.count());
		}

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

		template <class T, class... ARGS>
		void append_filter(ARGS&&... args)
		{
			if (m_top_bev)
				_append_filter(std::unique_ptr<filter_handler>(new T(std::forward<ARGS>(args)...)));
		}

		template <class T>
		void set_event_handler(T&& handler) { m_event_handler = std::forward<T>(handler); }
		const event_handler& get_event_handler() const { return m_event_handler; }

		void set_extra_data(void* extra) { m_extra_data = extra; }
		void* get_extra_data() const { return m_extra_data; }

		void close() { tcp_connection(std::move(*this)); }

		void swap(tcp_connection& rhs)
		{
			using std::swap;
			swap(m_top_bev, rhs.m_top_bev);
			swap(m_base_bev, rhs.m_base_bev);
			swap(m_token_bucket, rhs.m_token_bucket);
			swap(m_event_handler, rhs.m_event_handler);
			swap(m_extra_data, rhs.m_extra_data);

			_reset_callbacks();
			rhs._reset_callbacks();
		}
		
	private:
		explicit tcp_connection(bufferevent* bev);

		void _set_timeouts(long read_sec, long read_usec, long write_sec, long write_usec);
		void _append_filter(std::unique_ptr<filter_handler> filter);
		void _reset_callbacks();

	private:
		bufferevent* m_top_bev;
		bufferevent* m_base_bev;
		std::shared_ptr<const token_bucket_cfg> m_token_bucket;
		event_handler m_event_handler;
		void* m_extra_data = nullptr;
	};

	inline void swap(tcp_connection& lhs, tcp_connection& rhs)
	{
		lhs.swap(rhs);
	}

} }

namespace std
{

	template <>
	struct less<ranger::event::tcp_connection>
	{
		using result_type = bool;
		using first_argument_type = ranger::event::tcp_connection;
		using second_argument_type = ranger::event::tcp_connection;

		result_type operator () (const first_argument_type& lhs, const second_argument_type& rhs) const
		{
			return lhs.file_descriptor() < rhs.file_descriptor();
		}
	};

	template <>
	struct equal_to<ranger::event::tcp_connection>
	{
		using result_type = bool;
		using first_argument_type = ranger::event::tcp_connection;
		using second_argument_type = ranger::event::tcp_connection;

		result_type operator () (const first_argument_type& lhs, const second_argument_type& rhs) const
		{
			return lhs.file_descriptor() == rhs.file_descriptor();
		}
	};

	template <>
	struct hash<ranger::event::tcp_connection>
	{
		using result_type = size_t;
		using argument_type = ranger::event::tcp_connection;

		result_type operator () (const argument_type& conn) const
		{
			return conn.file_descriptor();
		}
	};

}

#endif	// RANGER_EVENT_TCP_CONNECTION_HPP
