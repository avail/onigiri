#include <stdinc.hpp>

#include <utils/static_initializer.hpp>


static int WINAPI NoWindowsHookExA(int, HOOKPROC, HINSTANCE, DWORD)
{
	return 1;
}

static onigiri::utils::static_initializer _([]()
{
	// disable SetWindowsHookExA bullshit
	MH_Initialize();
	MH_CreateHookApi(L"user32.dll", "SetWindowsHookExA", NoWindowsHookExA, NULL);
	MH_EnableHook(MH_ALL_HOOKS);
});
