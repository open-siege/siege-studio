#ifndef MESSAGE_HANDLER_HPP
#define MESSAGE_HANDLER_HPP

#include <siege/platform/win/desktop/window.hpp>
#include <siege/platform/win/core/com/base.hpp>
#include <oleacc.h>

namespace siege::extension
{
	struct MessageHandler : win32::window_ref
	{
		win32::com::com_ptr<IDispatch> script_host;

		MessageHandler(win32::hwnd_t self, const CREATESTRUCTW& args) : win32::window_ref(self), script_host((IDispatch*)args.lpCreateParams)
		{
		}

		std::optional<LRESULT> wm_get_object(win32::get_object_message message)
		{
			if (message.object_id == OBJID_NATIVEOM)
			{
				auto temp = script_host;

				auto result = LresultFromObject(__uuidof(IDispatch), message.flags, temp.get());

				return result;
			}

			return std::nullopt;
		}
	};

}


#endif