#include <stdinc.hpp>

#include <utils/static_initializer.hpp>

static int WINAPI no_windows_hook_ex_a(int, HOOKPROC, HINSTANCE, DWORD)
{
	return 1;
}

static onigiri::utils::static_initializer _([]()
{
	onigiri::utils::iat(L"user32.dll", "SetWindowsHookExA", no_windows_hook_ex_a);

	// never hide console >:(
	hook::nop(hook::get_pattern("FF 15 ? ? ? ? E8 ? ? ? ? 65 48 8B 0C 25 ? ? ? ? 8B 05 ? ? ? ? 48 8B 04 C1 BA ? ? ? ? 83 24 02 00 E8"), 6);
});
