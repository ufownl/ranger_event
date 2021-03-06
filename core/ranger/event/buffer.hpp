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

#ifndef RANGER_EVENT_BUFFER_HPP
#define RANGER_EVENT_BUFFER_HPP

#ifndef SWIG
#include <string>

struct evbuffer;
#endif	// !SWIG

namespace ranger { namespace event {

class buffer {
public:
	buffer();
	~buffer();

#ifndef SWIG
	buffer& operator = (const buffer&) = delete;

	buffer(buffer&& rhs) noexcept
		: m_buf(rhs.m_buf)
		, m_flag(rhs.m_flag) {
		rhs.m_buf = nullptr;
	}

	buffer& operator = (buffer&& rhs) noexcept {
		if (this != &rhs) {
			auto tmp = std::move(rhs);
			swap(tmp);
		}

		return *this;
	}
#endif	// !SWIG

	bool append(const void* src, size_t len);
	bool append(buffer& src);
#ifndef SWIG
	int printf(const char* fmt, ...);
	int vprintf(const char* fmt, va_list ap);
#endif	// !SWIG

	int remove(void* dst, size_t len);
	int remove(buffer& dst, size_t len);
	std::string readln();
	ssize_t copyout(void* dst, size_t len) const;
	bool drain(size_t len);

	size_t search(const void* src, size_t len, size_t pos = 0) const;
	size_t size() const;

#ifndef SWIG
	void swap(buffer& rhs) noexcept {
		if (this != &rhs) {
			using std::swap;
			swap(m_buf, rhs.m_buf);
			swap(m_flag, rhs.m_flag);
		}
	}

#ifdef RANGER_INTERNAL
public:
#else
private:
#endif	// RANGER_INTERNAL
	explicit buffer(evbuffer* buf)
		: m_buf(buf)
		, m_flag(false) {
		// nop
	}

	evbuffer* backend() const {
		return m_buf;
	}

#ifdef RANGER_SWIG_INTERNAL
public:
#else
private:
#endif
	// Only for SWIG
	buffer(const buffer& rhs)
		: m_buf(rhs.m_buf)
		, m_flag(false) {
		// nop
	}

private:
	evbuffer* m_buf;
	bool m_flag;
#endif	// !SWIG
};

#ifndef SWIG
inline void swap(buffer& lhs, buffer& rhs) noexcept {
	lhs.swap(rhs);
}
#endif	// !SWIG

} }

#endif	// RANGER_EVENT_BUFFER_HPP
