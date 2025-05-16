#include <stdinc.hpp>

#include <Hooking.h>

#include <utils/static_initializer.hpp>

static int WINAPI NoWindowsHookExA(int, HOOKPROC, HINSTANCE, DWORD)
{
	return 1;
}

void UnprotectPages()
{
	onigiri::services::logger::info("Unprotecting pages..");

	auto modBase = (uintptr_t)GetModuleHandle(nullptr);
	for (uint64_t i = modBase; i < modBase + 0x146000000;)
	{
		MEMORY_BASIC_INFORMATION mbi;
		VirtualQuery((void*)i, &mbi, sizeof(mbi));

		bool noRights = (mbi.Protect & PAGE_GUARD) || (mbi.Protect & PAGE_NOACCESS);

		if (!noRights)
		{
			DWORD oldProt;
			auto res = VirtualProtect((void*)i, 512, PAGE_EXECUTE_READWRITE, &oldProt);
			if (!res)
			{
				onigiri::services::logger::info("Virtualprotect {} - {:X}", res, oldProt);
			}
		}

		i += mbi.RegionSize;
	}

	onigiri::services::logger::info("DONE Unprotecting pages..");
}

static onigiri::utils::static_initializer _([]()
{
	UnprotectPages();

	onigiri::utils::iat(L"user32.dll", "SetWindowsHookExA", NoWindowsHookExA);

	// never hide console >:(
	hook::nopVP(hook::get_pattern("74 0B 48 89 C1 31 D2"), 7);

	// skips intro video
	if (onigiri::utils::storage::SkipIntro)
	{
		onigiri::services::logger::info("patching intro video~");
		hook::putVP(hook::get_address<char*>(hook::get_pattern("84 C9 0F ? ? ? ? ? C6 44 24 20 01", 30)), "nah");

		DWORD threadId;
		CreateThread(nullptr, 0, [](PVOID) -> DWORD
		{
			Sleep(5000);

			onigiri::services::logger::info("restoring bik~");
			hook::putVP(hook::get_address<char*>(hook::get_pattern("84 C9 0F ? ? ? ? ? C6 44 24 20 01", 30)), "bik");

			return 0;
		}, nullptr, 0, &threadId);
	}

	// skips legal
	if (onigiri::utils::storage::SkipLegal)
	{
		onigiri::services::logger::info("patching legal~");
		hook::nopVP(hook::get_pattern("E9 ? ? ? ? 84 C9 74 66", -12), 2);
	}

	// skip startup banner
	if (onigiri::utils::storage::SkipSplash)
	{
		onigiri::services::logger::info("patching splash~");
		hook::putVP(hook::get_pattern("56 53 48 83 EC ? 48 83 3D"), 0xC3C03148); // xor rax, rax; retn
	}

	// patch "rpf.cache" out
	hook::putVP(hook::get_address<char*>(hook::get_pattern("48 8D 74 24 70 48 89 F2 41 B8 00 01 00 00", -23)), "nah.cache");

	// return some function that debug prints
	hook::putVP(hook::get_pattern("56 57 48 81 EC ? ? ? ? 4C 89 CE 4C 89 C7"), 0xC3C03148); // xor rax, rax; retn

	// no launcher, lol
	//hook::putVP<uint8_t>(hook::get_pattern("75 ? B9 ? ? ? ? B2 ? E8 ? ? ? ? B8"), 0xEB);

	// patch streamer

	// heap size
	{
		int* loc = hook::get_pattern<int32_t>("BA ? ? ? ? 45 31 C0 41 B1 ? E8 ? ? ? ? 48 8D 0D", 1);
		auto vanillaHeapSize = *loc / 1024 / 1024;
		auto heapSize = 1024 * 1024 * 1024;

		DWORD oldProtect;
		VirtualProtect(loc, 4, PAGE_EXECUTE_READWRITE, &oldProtect);

		onigiri::services::logger::info("vanilla heap size: {}MB", vanillaHeapSize);

		*loc = heapSize;

		onigiri::services::logger::info("modified heap size: {}MB", heapSize / 1024 / 1024);

		VirtualProtect(loc, 4, oldProtect, &oldProtect);
	}

	// TODO: relocate this.
	// patch sm_Collections limit
	/*{
		// originally: 1a58 (6744)
		auto newLimit = 9999;

		// fiPackfile ctor
		{
			auto location = hook::get_pattern("48 81 F9 ? ? ? ? 74 ? 48 83 3C CA", 3);
			hook::putVP(location, newLimit);
		}

		// not inlined thing
		{
			auto location = hook::get_address<void*>(hook::get_pattern("C1 EE ? 3B 35", 5));
			hook::putVP(location, newLimit);
		}
	}*/
});
