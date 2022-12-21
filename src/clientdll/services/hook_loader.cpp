#include <stdinc.hpp>

#include <services/hook_loader.hpp>

namespace onigiri::services
{
	void __stdcall hook_loader::get_startup_info_w_hook(_Out_ LPSTARTUPINFOW lpStartupInfo)
	{
		hook_loader::get_startup_info_w_orig(lpStartupInfo);

		if (hook_loader::did_run)
		{
			return;
		}

		hook_loader::did_run = true;

		hook::set_base();

		onigiri::services::logger::debug("initializing function stubs");
		HookFunction::RunAll();
		onigiri::services::logger::debug("done initializing function stubs~");

		onigiri::services::logger::debug("running static initializers");
		onigiri::utils::static_initializer::init();
		onigiri::services::logger::debug("done running static initializers~");
	}

	void hook_loader::initialize()
	{
		hook_loader::get_startup_info_w_orig = utils::iat(L"kernel32.dll", "GetStartupInfoW", hook_loader::get_startup_info_w_hook);
	}
}
