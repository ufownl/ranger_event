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

#include "buffer.hpp"
#include <event2/buffer.h>
#include <stdlib.h>

namespace ranger { namespace event {

	buffer::buffer()
		: m_buf(evbuffer_new())
		, m_flag(true)
	{
		if (!m_buf)
			throw std::runtime_error("evbuffer create failed.");

		//evbuffer_enable_locking(m_buf, nullptr);
	}

	buffer::~buffer()
	{
		if (m_buf && m_flag)
			evbuffer_free(m_buf);
	}

	bool buffer::append(const void* src, size_t len)
	{
		if (!m_buf)
			return false;

		return evbuffer_add(m_buf, src, len) == 0;
	}

	bool buffer::append(buffer& src)
	{
		if (!m_buf)
			return false;

		return evbuffer_add_buffer(m_buf, src.m_buf) == 0;
	}

	int buffer::printf(const char* fmt, ...)
	{
		int ret = -1;

		if (m_buf)
		{
			va_list ap;
			va_start(ap, fmt);
			ret = evbuffer_add_vprintf(m_buf, fmt, ap);
			va_end(ap);
		}

		return ret;
	}

	int buffer::vprintf(const char* fmt, va_list ap)
	{
		if (!m_buf)
			return -1;

		return evbuffer_add_vprintf(m_buf, fmt, ap);
	}

	int buffer::remove(void* dst, size_t len)
	{
		if (!m_buf)
			return -1;

		return evbuffer_remove(m_buf, dst, len);
	}

	int buffer::remove(buffer& dst, size_t len)
	{
		if (!m_buf)
			return -1;

		return evbuffer_remove_buffer(m_buf, dst.m_buf, len);
	}

	std::string buffer::readln()
	{
		std::string ret;

		if (m_buf)
		{
			size_t len = 0;
			std::unique_ptr<char, decltype(&free)> line(evbuffer_readln(m_buf, &len, EVBUFFER_EOL_CRLF), free);
			if (len > 0) ret = line.get();
		}

		return ret;
	}

	ssize_t buffer::copyout(void* dst, size_t len) const
	{
		return evbuffer_copyout(m_buf, dst, len);
	}

	bool buffer::drain(size_t len)
	{
		return evbuffer_drain(m_buf, len) == 0;
	}

	size_t buffer::search(const void* src, size_t len, size_t pos /* = 0 */) const
	{
		evbuffer_ptr ptr;
		evbuffer_ptr_set(m_buf, &ptr, pos, EVBUFFER_PTR_SET);
		ptr = evbuffer_search(m_buf, static_cast<const char*>(src), len, &ptr);
		return ptr.pos;
	}

	size_t buffer::size() const
	{
		return evbuffer_get_length(m_buf);
	}

} }
