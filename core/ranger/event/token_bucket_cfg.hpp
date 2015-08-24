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

#ifndef RANGER_EVENT_TOKEN_BUCKET_CFG_HPP
#define RANGER_EVENT_TOKEN_BUCKET_CFG_HPP

#ifndef SWIG
#include <memory>
#include <chrono>

struct ev_token_bucket_cfg;
#endif	// !SWIG

namespace ranger { namespace event {

class token_bucket_cfg {
public:
	~token_bucket_cfg();

#ifndef SWIG
	token_bucket_cfg(const token_bucket_cfg&) = delete;
	token_bucket_cfg& operator = (const token_bucket_cfg&) = delete;

	template <class Rep, class Period>
	static std::shared_ptr<const token_bucket_cfg>
	create(	size_t read_rate, size_t read_burst,
			size_t write_rate, size_t write_burst,
			const std::chrono::duration<Rep, Period>& period = std::chrono::seconds(0)) {
		auto sec = std::chrono::duration_cast<std::chrono::seconds>(period);
		auto usec = std::chrono::duration_cast<std::chrono::microseconds>(period - sec);
		return create_impl(read_rate, read_burst, write_rate, write_burst, sec.count(), usec.count());
	}

#ifdef RANGER_INTERNAL
public:
#else
private:
#endif	// RANGER_INTERNAL
	token_bucket_cfg(	size_t read_rate, size_t read_burst,
						size_t write_rate, size_t write_burst,
						long sec, long usec);

	ev_token_bucket_cfg* backend() const {
		return m_cfg;
	}

private:
	static std::shared_ptr<const token_bucket_cfg>
	create_impl(size_t read_rate, size_t read_burst,
				size_t write_rate, size_t write_burst,
				long sec, long usec);

	ev_token_bucket_cfg* m_cfg;
#endif	// !SWIG
};

} }

#endif	// RANGER_EVENT_TOKEN_BUCKET_CFG_HPP
