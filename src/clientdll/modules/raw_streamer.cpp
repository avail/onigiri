#include <stdinc.hpp>

#include <utils/static_initializer.hpp>

// result, file, quitOnBadVersion, relativePath, quitIfMissing, overlayIfExists
static hook::cdecl_stub<uint32_t* (uint32_t*, const char*, bool, const char*, bool, bool)> rage__strPackfileManager__RegisterIndividualFile([]()
{
	return hook::get_pattern("41 57 41 56 56 57 55 53 48 83 EC ? 4C 89 CF 48 89 D6");
});

void RegisterAssets()
{
	auto reg = [](const std::string& path, const std::string& name)
	{
		uint32_t file_id = -1;
		rage__strPackfileManager__RegisterIndividualFile(&file_id, path.c_str(), false, name.c_str(), false, true);

		if (file_id == -1)
		{
			onigiri::services::logger::info("Failed registering asset {}", name);
		}
		else
		{
			onigiri::services::logger::info("Successfully registered asset {}", name);
		}
	};

	auto path = onigiri::utils::storage::get_game_path("onigiri\\assets\\fxdecal.ytd");
	if (std::filesystem::exists(path))
	{
		reg(path, "fxdecal");
	}
}

void (*CGtaRenderThreadGameInterface__PerformSafeModeOperations)(void* a1);
void CGtaRenderThreadGameInterface__PerformSafeModeOperationsHook(void* a1)
{
	CGtaRenderThreadGameInterface__PerformSafeModeOperations(a1);

	static bool didRun = false;
	if (!didRun)
	{
		onigiri::services::logger::info("NVENVENVE RUNNING");
		RegisterAssets();

		didRun = true;
	}
}

static onigiri::utils::static_initializer _([]()
{
	onigiri::services::logger::info("streaming init~");

	//hook::put<uint8_t>(hook::get_pattern("66 78 64 65 63 61 6C 00", 3), 'f');

	//MH_Initialize();
	//MH_CreateHook(hook::get_pattern("56 57 53 48 83 EC ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 8B 05 ? ? ? ? 65 48 8B"),
		//CGtaRenderThreadGameInterface__PerformSafeModeOperationsHook, (void**)&CGtaRenderThreadGameInterface__PerformSafeModeOperations);
	//MH_EnableHook(MH_ALL_HOOKS);
});
