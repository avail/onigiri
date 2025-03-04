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
	hook::nop(hook::get_pattern("74 0B 48 89 C1 31 D2"), 7);
});
