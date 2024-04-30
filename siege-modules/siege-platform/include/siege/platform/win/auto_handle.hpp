#ifndef AUTO_HANDLE_HPP
#define AUTO_HANDLE_HPP

#include <memory>


namespace win32
{
	template <typename THandle, typename TDeleter>
	struct auto_handle : std::unique_ptr<std::decay_t<decltype(*std::declval<THandle>())>, TDeleter>
	{
		using base = std::unique_ptr<std::decay_t<decltype(*std::declval<THandle>())>, TDeleter>;
		using base::base;

		operator THandle()
		{
			return this->get();
		}
	};

}

#endif