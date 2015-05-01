#ifndef RANGER_UTIL_RW_LOCK_HPP
#define RANGER_UTIL_RW_LOCK_HPP

#include <mutex>
#include <condition_variable>

namespace ranger { namespace util {

	class rw_lock
	{
	public:
		rw_lock() = default;

		rw_lock(const rw_lock&) = delete;
		rw_lock& operator = (const rw_lock&) = delete;

		void read_lock()
		{
			std::lock_guard<std::mutex> lock(m_mtx);
			++m_rlc;
		}

		void read_unlock()
		{
			std::unique_lock<std::mutex> lock(m_mtx);
			if (--m_rlc == 0)
			{
				lock.unlock();
				m_cv.notify_all();
			}
		}

		void write_lock()
		{
			std::unique_lock<std::mutex> lock(m_mtx);
			m_cv.wait(lock, [this] { return m_rlc == 0; });
			lock.release();
		}

		void write_unlock()
		{
			m_mtx.unlock();
		}

	private:
		std::mutex m_mtx;
		std::condition_variable m_cv;
		size_t m_rlc = 0;
	};

} }

#endif	// RANGER_UTIL_RW_LOCK_HPP
