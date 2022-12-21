#pragma once

namespace onigiri::services
{
	class hook_loader final
	{
	public:
		static void initialize();

	private:
		static inline bool did_run = false;

		static void __stdcall get_startup_info_w_hook(_Out_ LPSTARTUPINFOW lpStartupInfo);
		static inline utils::function<decltype(get_startup_info_w_hook)> get_startup_info_w_orig;
	};
}
