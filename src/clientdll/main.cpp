#include <stdinc.hpp>

#include <iostream>

#include <modules/openness.hpp>
#include <services/hook_loader.hpp>

#define RUN_THREAD 1

void OnAttach(HMODULE hModule)
{

#if RUN_THREAD
	static auto hello_it_is_i = hModule;
	static DWORD our_id;

	static HANDLE thread = CreateThread(nullptr, 0, [](PVOID) -> DWORD
	{
#endif
		/*while (!FindWindow(L"grcWindow", L"Grand Theft Auto V"))
		{
			std::this_thread::sleep_for(100ms);
		}*/

		onigiri::services::logger::initialize();

		onigiri::utils::storage::init();
		onigiri::utils::init();

		onigiri::services::logger::info("              __          __         __");
		onigiri::services::logger::info("  ___    ___ /\\_\\     __ /\\_\\  _ __ /\\_\\");
		onigiri::services::logger::info(" / __`\\/' _ `\\/\\ \\  /'_ `\\/\\ \\/\\`'__\\/\\ \\");
		onigiri::services::logger::info("/\\ \\L\\ /\\ \\/\\ \\ \\ \\/\\ \\L\\ \\ \\ \\ \\ \\/ \\ \\ \\");
		onigiri::services::logger::info("\\ \\____\\ \\_\\ \\_\\ \\_\\ \\____ \\ \\_\\ \\_\\  \\ \\_\\");
		onigiri::services::logger::info(" \\/___/ \\/_/\\/_/\\/_/\\/___L\\ \\/_/\\/_/   \\/_/");
		onigiri::services::logger::info("                      /\\____/");
		onigiri::services::logger::info("                      \\_/__/");
		onigiri::services::logger::info("       NaturalVision Enhanced (c) RazedMods & avail");
		onigiri::services::logger::info("            https://patreon.com/razedmods");

		onigiri::services::logger::info("onigiri started~");

		onigiri::services::hook_loader::initialize();

		// hm?
		return 0;

		g_running = true;

		while (g_running)
		{
			// i press this by accident all the time
			/*if (GetAsyncKeyState(VK_DELETE) & 0x1)
			{
				g_running = false;
			}*/

			std::this_thread::sleep_for(250ms);
		}

		onigiri::services::logger::info("onigiri shutting down~");

		onigiri::utils::shutdown();
		onigiri::services::logger::shutdown();

#if RUN_THREAD

		CloseHandle(thread);
		FreeLibraryAndExitThread(hello_it_is_i, 0);
	}, nullptr, 0, &our_id);
#endif
}

bool __stdcall DllMain(HMODULE hModule, DWORD reason, PVOID)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hModule);
		OnAttach(hModule);
		break;
	}

	case DLL_PROCESS_DETACH:
	{
		ExitProcess(0);
		break;
	}
	}

	return true;
}
