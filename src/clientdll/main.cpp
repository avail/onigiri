#include <stdinc.hpp>

#include <modules/reflections.hpp>
#include <services/hook_loader.hpp>

bool __stdcall DllMain(HMODULE hModule, DWORD reason, PVOID)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hModule);

		static auto hello_it_is_i = hModule;
		static DWORD our_id;

		static HANDLE thread = CreateThread(nullptr, 0, [](PVOID) -> DWORD
		{
			/*while (!FindWindow(L"grcWindow", L"Grand Theft Auto V"))
			{
				std::this_thread::sleep_for(100ms);
			}*/

			onigiri::services::logger::initialize();

			onigiri::utils::storage::init();
			onigiri::utils::init();

			onigiri::services::logger::info("onigiri started~");

			/*onigiri::scripthook::ScriptEngine::Initialize();

			onigiri::scripthook::ScriptEngine::CreateThread(&onigiri::scripthook::g_ScriptManagerThread);*/

			onigiri::services::hook_loader::initialize();

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

			// minhook unpatch + restore memory of that 1 location
			//onigiri::modules::reflections::shutdown();

			//onigiri::scripthook::g_ScriptManagerThread.FreeScripts();
			//onigiri::scripthook::ScriptEngine::RemoveAllThreads();

			onigiri::utils::shutdown();
			onigiri::services::logger::shutdown();

			CloseHandle(thread);
			FreeLibraryAndExitThread(hello_it_is_i, 0);
		}, nullptr, 0, &our_id);
	}

	return true;
}
