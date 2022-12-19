#include <stdinc.hpp>

#include <gta/fiPackfile.hpp>

#include <utils/static_initializer.hpp>
#include <utils/Hooking.Invoke.h>
#include <utils/Hooking.h>
#include <utils/Hooking.IAT.h>

rage::fiDeviceLocal* platformDevice;
rage::fiDeviceLocal* platformDeviceCRC;
rage::fiDeviceLocal* platform2Device;
rage::fiDeviceLocal* platform2DeviceCRC;
rage::fiDeviceLocal* commonDevice;
rage::fiDeviceLocal* commonDeviceCRC;
rage::fiDeviceLocal* dlcDevice;

bool IsCustomDevice(rage::fiDeviceLocal* dev)
{
	if (dev == platformDevice ||
		dev == platformDeviceCRC ||
		dev == platform2Device ||
		dev == platform2DeviceCRC ||
		dev == commonDevice ||
		dev == commonDeviceCRC ||
		dev == dlcDevice)
	{
		return true;
	}

	return false;
}

void(*g_origInitialMount)();
void InitialMountHook()
{
	g_origInitialMount();

	std::filesystem::path cwd = std::filesystem::current_path() / "onigiri/";
	cwd.make_preferred();

	onigiri::services::logger::debug("mods path: %s", cwd.string().c_str());

	rage::fiDeviceLocal* rootDevice = new rage::fiDeviceLocal();
	rootDevice->SetPath(cwd.string().c_str(), true, nullptr);

	if (rootDevice->Mount("onigiri:/"))
	{
		onigiri::services::logger::debug("root device mounted~");
	}

	platformDevice = new rage::fiDeviceLocal();
	platformDeviceCRC = new rage::fiDeviceLocal();
	platform2Device = new rage::fiDeviceLocal();
	platform2DeviceCRC = new rage::fiDeviceLocal();
	commonDevice = new rage::fiDeviceLocal();
	commonDeviceCRC = new rage::fiDeviceLocal();
	dlcDevice = new rage::fiDeviceLocal();

	platformDevice->SetPath("onigiri:/platform/", true, nullptr);
	platformDevice->Mount("platform:/");

	platformDeviceCRC->SetPath("onigiri:/platform/", true, nullptr);
	platformDeviceCRC->Mount("platformcrc:/");

	platform2Device->SetPath("onigiri:/platform2/", true, nullptr);
	platform2Device->Mount("platform2:/");

	platform2DeviceCRC->SetPath("onigiri:/platform2/", true, nullptr);
	platform2DeviceCRC->Mount("platform2crc:/");

	commonDevice->SetPath("onigiri:/common/", true, nullptr);
	commonDevice->Mount("common:/");

	commonDeviceCRC->SetPath("onigiri:/common/", true, nullptr);
	commonDeviceCRC->Mount("commoncrc:/");

	dlcDevice->SetPath("onigiri:/dlcpacks/", true, nullptr);
	dlcDevice->Mount("dlcpacks:/");
}

static hook::cdecl_stub<rage::fiDevice*(const char*, bool)> GetDeviceHook([]()
{
	return hook::get_pattern("48 89 5C 24 ? 88 54 24 10 55 56 57 41 54 41 55 41 56 41 57 48 83 EC 20 48 8D 15 ? ? ? ? 41 B8");
});

char(*g_origOpenArchive)(rage::fiPackfile*, const char*, char, int32_t, __int64);
char OpenArchiveHook(rage::fiPackfile* a1, const char* path, char a3, int32_t type, __int64 a5)
{
	auto device = GetDeviceHook(path, true);

	if (device && IsCustomDevice((rage::fiDeviceLocal*)device))
	{
		if (device->GetAttributes(path) & FILE_ATTRIBUTE_DIRECTORY)
		{
			onigiri::services::logger::debug("archive %s is a directory~", path);
			type = 2;
		}

		onigiri::services::logger::debug("opening archive %s %d %d", path, a3, type);
	}

	return g_origOpenArchive(a1, path, a3, type, a5);
}

static void(*dataFileMgr__loadDat)(void*, const char*, bool);
static void LoadDats(void* dataFileMgr, const char* name, bool enabled)
{
	// load before-level metas
	{
		onigiri::services::logger::debug("loading before-level metas~");
		auto meh = std::filesystem::current_path() / "onigiri" / "before_level.xml";
		dataFileMgr__loadDat(dataFileMgr, &meh.string()[0], enabled);
	}

	// load the level
	onigiri::services::logger::debug("loading level~");
	dataFileMgr__loadDat(dataFileMgr, name, enabled);

	// load after-level metas
	{
		onigiri::services::logger::debug("loading after-level metas~");
		auto meh = std::filesystem::current_path() / "onigiri" / "after_level.xml";
		dataFileMgr__loadDat(dataFileMgr, &meh.string()[0], enabled);
	}
}

static bool (*g_origParamToInt)(void* param, int* value);
static bool ParamToInt_Threads(void* param, int* value)
{
	bool rv = g_origParamToInt(param, value);

	if (!rv)
	{
		*value = std::min(*value, 4);
	}

	return rv;
}

static HookFunction __([]()
{
	onigiri::services::logger::info("applying custom device hooks");

	MH_Initialize();
	MH_CreateHook(hook::get_pattern("48 8B C4 48 89 58 10 48 89 70 18 48 89 78 20 55 41 54 41 55 41 56 41 57 48 8D 68 98 48 81 EC ? ? ? ? 41 8B F9 4C 8B E2 48 8B D9 4C 8B CA 48 8D 05"), OpenArchiveHook, (void**)&g_origOpenArchive);
	MH_EnableHook(MH_ALL_HOOKS);

	hook::set_call(&g_origInitialMount, hook::pattern("48 03 C3 44 88 34 38 66 01 1D").count(1).get(0).get<void>(0xE));
	hook::call(hook::pattern("48 03 C3 44 88 34 38 66 01 1D").count(1).get(0).get<void>(0xE), InitialMountHook);

	{
		void* location = hook::pattern("E8 ? ? ? ? 48 8B 0D ? ? ? ? 41 B0 01 48 8B D3").count(1).get(0).get<void>(18);
		hook::set_call(&dataFileMgr__loadDat, location);
		hook::call(location, LoadDats);
	}

	// increase non-DLC fiDevice mount limit
	{
		auto location = hook::get_pattern<int>("C7 05 ? ? ? ? 64 00 00 00 48 8B", 6);
		hook::put<int>(location, *location * 15); // '1500' mount limit now, instead of '500'
	}

	// don't sort update:/ relative devices before ours
	hook::nop(hook::pattern("C6 80 F0 00 00 00 01 E8 ? ? ? ? E8").count(1).get(0).get<void>(12), 5);

	// don't crash (forget to call rage::fiDevice::Unmount) on failed DLC text reads
	{
		auto location = hook::get_pattern("41 8B D6 E9 7C 02 00 00", 4);
		*(int*)location -= 0x12;
	}

	// increase reserved physical memory amount threefold (to ~900 MB)
	hook::put<uint32_t>(hook::get_pattern("48 81 C1 00 00 00 12 48 89 0D", 3), 0x36000000);

	// limit max worker threads to 4 (since on high-core-count systems this leads
	// to a lot of overhead when there's a blocking wait)
	{
		auto location = hook::get_pattern("89 05 ? ? ? ? E8 ? ? ? ? 48 8D 3D ? ? ? ? 48 63", 6);
		hook::set_call(&g_origParamToInt, location);
		hook::call(location, ParamToInt_Threads);
	}

	// extend grcResourceCache pool a bit
	{
		auto location = hook::get_pattern<char>("BA 00 00 05 00 48 8B C8 44 88");
		hook::put<uint32_t>(location + 1, 0xA0000);
		hook::put<uint32_t>(location + 23, 0xA001B);
	}

	// increase allocator amount
	auto location = hook::get_pattern("41 B8 00 00 00 40 48 8B D5 89", 2);

	auto allocatorReservation = 0x7FFFFFFF;;
	if (allocatorReservation)
	{
		hook::put<uint32_t>(location, allocatorReservation);
	}

	// increase the heap size for allocator 0
	hook::put<uint32_t>(hook::get_pattern("83 C8 01 48 8D 0D ? ? ? ? 41 B1 01 45 33 C0", 17), 700 * 1024 * 1024); // 700 MiB, default in 323 was 412 MiB
});
