#ifndef RANGER_UTIL_SCOPE_GUARD_HPP
#define RANGER_UTIL_SCOPE_GUARD_HPP

#include <functional>

namespace ranger { namespace util {

	class scope_guard
	{
	public:
		template <class T>
		explicit scope_guard(T&& handler)
			: m_exit_handler(std::forward<T>(handler))
			, m_dismiss(false)
		{
		}

		~scope_guard()
		{
			if (!m_dismiss)
				m_exit_handler();
		}

		scope_guard(const scope_guard&) = delete;
		scope_guard& operator = (const scope_guard&) = delete;

		void dismiss()
		{
			m_dismiss = true;
		}

	private:
		std::function<void()> m_exit_handler;
		bool m_dismiss;
	};

} }

#endif	// RANGER_UTIL_SCOPE_GUARD_HPP
