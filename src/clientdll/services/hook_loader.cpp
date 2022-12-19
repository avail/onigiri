#include <stdinc.hpp>

#include <services/hook_loader.hpp>

#include <utils/HookFunction.h>
#include <utils/Hooking.h>

namespace onigiri::services
{
	void __stdcall hook_loader::get_startup_info_w_hook(_Out_ LPSTARTUPINFOW lpStartupInfo)
	{
		hook_loader::orig_startup_info_hook_w(lpStartupInfo);

		if (hook_loader::did_run)
		{
			return;
		}

		hook_loader::did_run = true;

		// never hide console >:(
		hook::nop(hook::get_pattern("FF 15 ? ? ? ? E8 ? ? ? ? 65 48 8B 0C 25 ? ? ? ? 8B 05 ? ? ? ? 48 8B 04 C1 BA ? ? ? ? 83 24 02 00 E8"), 6);

		hook::set_base();

		onigiri::services::logger::debug("running hook functions");
		HookFunction::RunAll();
		onigiri::services::logger::debug("done running hook functions~");

		onigiri::services::logger::debug("running static initializers");
		onigiri::utils::static_initializer::init();
		onigiri::services::logger::debug("done running static initializers~");
	}

	void hook_loader::initialize()
	{
		MH_Initialize();
		MH_CreateHookApi(L"kernel32.dll", "GetStartupInfoW", hook_loader::get_startup_info_w_hook, (void**)&hook_loader::orig_startup_info_hook_w);
		MH_EnableHook(MH_ALL_HOOKS);
	}
}
