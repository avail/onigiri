#include <stdinc.hpp>

#include <gta/fiPackfile.hpp>

#include <utils/static_initializer.hpp>

#include <modules/custom_device.hpp>

namespace onigiri::modules
{
	static hook::cdecl_stub<rage::fiDevice* (const char*, bool)> GetDeviceHook([]()
	{
		return hook::get_pattern("48 89 5C 24 ? 88 54 24 10 55 56 57 41 54 41 55 41 56 41 57 48 83 EC 20 48 8D 15 ? ? ? ? 41 B8");
	});

	char custom_device::open_archive_hook(rage::fiPackfile* a1, const char* path, char a3, int32_t type, __int64 a5)
	{
		auto device = GetDeviceHook(path, true);

		if (device && custom_device::is_custom_device((rage::fiDeviceLocal*)device))
		{
			if (device->GetAttributes(path) & FILE_ATTRIBUTE_DIRECTORY)
			{
				onigiri::services::logger::debug("archive %s is a directory~", path);
				type = 2;
			}

			onigiri::services::logger::debug("opening archive %s %d %d", path, a3, type);
		}

		return custom_device::open_archive(a1, path, a3, type, a5);
	}

	void custom_device::initial_mount_hook()
	{
		custom_device::initial_mount();

		std::filesystem::path cwd = std::filesystem::current_path() / "onigiri/";
		cwd.make_preferred();

		services::logger::debug("mods path: {}", cwd.string());

		rage::fiDeviceLocal* root_device = new rage::fiDeviceLocal();
		root_device->SetPath(cwd.string().c_str(), true, nullptr);

		if (root_device->Mount("onigiri:/"))
		{
			services::logger::debug("root device mounted~");
		}

		platform_device = new rage::fiDeviceLocal();
		platform_device_crc = new rage::fiDeviceLocal();
		platform2_device = new rage::fiDeviceLocal();
		platform2_device_crc = new rage::fiDeviceLocal();
		common_device = new rage::fiDeviceLocal();
		common_device_crc = new rage::fiDeviceLocal();
		dlc_device = new rage::fiDeviceLocal();

		platform_device->SetPath("onigiri:/platform/", true, nullptr);
		platform_device->Mount("platform:/");

		platform_device_crc->SetPath("onigiri:/platform/", true, nullptr);
		platform_device_crc->Mount("platformcrc:/");

		platform2_device->SetPath("onigiri:/platform2/", true, nullptr);
		platform2_device->Mount("platform2:/");

		platform2_device_crc->SetPath("onigiri:/platform2/", true, nullptr);
		platform2_device_crc->Mount("platform2crc:/");

		common_device->SetPath("onigiri:/common/", true, nullptr);
		common_device->Mount("common:/");

		common_device_crc->SetPath("onigiri:/common/", true, nullptr);
		common_device_crc->Mount("commoncrc:/");

		dlc_device->SetPath("onigiri:/dlcpacks/", true, nullptr);
		dlc_device->Mount("dlcpacks:/");
	}

	void custom_device::load_level_metas_hook(void* data_file_manager, const char* name, bool enabled)
	{
		{
			services::logger::debug("loading before-level metas~");
			auto meh = std::filesystem::current_path() / "onigiri" / "before_level.xml";
			custom_device::load_level_metas(data_file_manager, &meh.string()[0], enabled);
		}

		services::logger::debug("loading level~");
		custom_device::load_level_metas(data_file_manager, name, enabled);

		{
			services::logger::debug("loading after-level metas~");
			auto meh = std::filesystem::current_path() / "onigiri" / "after_level.xml";
			custom_device::load_level_metas(data_file_manager, &meh.string()[0], enabled);
		}
	}

	bool custom_device::set_thread_count_hook(void* param, int* value)
	{
		bool rv = custom_device::set_thread_count(param, value);

		if (!rv)
		{
			*value = std::min(*value, 4);
		}

		return rv;
	}

	STATICALLY_INITIALIZE(custom_device)([]()
	{
		services::logger::info("applying custom device hooks");

		custom_device::open_archive = utils::detour(hook::get_pattern("48 8B C4 48 89 58 10 48 89 70 18 48 89 78 20 55 41 54 41 55 41 56 41 57 48 8D 68 98 48 81 EC ? ? ? ? 41 8B F9 4C 8B E2 48 8B D9 4C 8B CA 48 8D 05"),
			custom_device::open_archive_hook);

		{
			auto location = hook::pattern("48 03 C3 44 88 34 38 66 01 1D").count(1).get(0).get<void>(0xE);
			custom_device::initial_mount = utils::call(location, custom_device::initial_mount_hook);
		}

		{
			void* location = hook::pattern("E8 ? ? ? ? 48 8B 0D ? ? ? ? 41 B0 01 48 8B D3").count(1).get(0).get<void>(18);
			custom_device::load_level_metas = utils::call(location, custom_device::load_level_metas_hook);
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
			custom_device::set_thread_count = utils::call(location, custom_device::set_thread_count_hook);
		}

		// extend grcResourceCache pool a bit
		{
			auto location = hook::get_pattern<char>("BA 00 00 05 00 48 8B C8 44 88");
			hook::put<uint32_t>(location + 1, 0xA0000);
			hook::put<uint32_t>(location + 23, 0xA001B);
		}

		// increase allocator amount
		{
			auto location = hook::get_pattern("41 B8 00 00 00 40 48 8B D5 89", 2);
			hook::put<uint32_t>(location, 0x7FFFFFFF);
		}

		// increase the heap size for allocator 0
		hook::put<uint32_t>(hook::get_pattern("83 C8 01 48 8D 0D ? ? ? ? 41 B1 01 45 33 C0", 17), 700 * 1024 * 1024); // 700 MiB, default in 323 was 412 MiB
	});

}
