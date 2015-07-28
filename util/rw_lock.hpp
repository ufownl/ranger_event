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

#ifndef RANGER_UTIL_RW_LOCK_HPP
#define RANGER_UTIL_RW_LOCK_HPP

#include <mutex>
#include <condition_variable>

namespace ranger { namespace util {

class rw_lock {
public:
	rw_lock() = default;

	rw_lock(const rw_lock&) = delete;
	rw_lock& operator = (const rw_lock&) = delete;

	void read_lock() {
		std::lock_guard<std::mutex> lock(m_mtx);
		++m_rlc;
	}

	void read_unlock() {
		std::unique_lock<std::mutex> lock(m_mtx);
		if (--m_rlc == 0) {
			lock.unlock();
			m_cv.notify_all();
		}
	}

	void write_lock() {
		std::unique_lock<std::mutex> lock(m_mtx);
		m_cv.wait(lock, [this] { return m_rlc == 0; });
		lock.release();
	}

	void write_unlock() {
		m_mtx.unlock();
	}

private:
	std::mutex m_mtx;
	std::condition_variable m_cv;
	size_t m_rlc = 0;
};

} }

#endif	// RANGER_UTIL_RW_LOCK_HPP
