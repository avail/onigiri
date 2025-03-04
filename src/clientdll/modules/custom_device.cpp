#include <stdinc.hpp>

#include <gta/fiPackfile.hpp>

#include <utils/static_initializer.hpp>

#include <modules/custom_device.hpp>


static hook::cdecl_stub<void(const char*, bool, rage::fiDevice*)> set_path([]()
{
	return hook::get_pattern("49 63 F5 42 0F BE 54 3E FF", -0x72);
});

static hook::cdecl_stub<bool*(rage::fiDevice*, const char*)> mount_wrap([]()
{
	return hook::get_pattern("44 0F B6 81 14 01 00 00", -0x1B);
});

namespace onigiri::modules
{
	static hook::cdecl_stub<rage::fiDevice* (const char*, bool)> GetDeviceHook([]()
	{
		return hook::get_pattern("48 81 EC 88 05 00 00 89 D6 49 89 CE", -12);
	});

	custom_device::fiDeviceRelative::fiDeviceRelative()
	{
	}

	void custom_device::fiDeviceRelative::SetPath(const char* relativeTo, rage::fiDevice* baseDevice, bool allowRoot)
	{
		set_path(relativeTo, allowRoot, baseDevice);
	}

	void custom_device::fiDeviceRelative::Mount(const char* mountPoint)
	{
		mount_wrap(GetDeviceHook(mountPoint, true), mountPoint);
	}

	char custom_device::open_archive_hook(rage::fiPackfile* a1, const char* path, char a3, int32_t type, __int64 a5)
	{
		auto device = GetDeviceHook(path, true);

		if (device && custom_device::is_custom_device((fiDeviceRelative*)device))
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

		fiDeviceRelative* root_device = new fiDeviceRelative();
		root_device->SetPath(cwd.string().c_str(), nullptr, true);
		root_device->Mount("onigiri:/");

		services::logger::debug("root device mounted~");
		
		platform_device = new fiDeviceRelative();
		platform_device->SetPath("onigiri:/platform/", nullptr, true);
		platform_device->Mount("platform:/");

		platform_device_crc = new fiDeviceRelative();
		platform_device_crc->SetPath("onigiri:/platform/", nullptr, true);
		platform_device_crc->Mount("platformcrc:/");

		platform2_device = new fiDeviceRelative();
		platform2_device->SetPath("onigiri:/platform2/", nullptr, true);
		platform2_device->Mount("platform2:/");

		platform2_device_crc = new fiDeviceRelative();
		platform2_device_crc->SetPath("onigiri:/platform2/", nullptr, true);
		platform2_device_crc->Mount("platform2crc:/");

		common_device = new fiDeviceRelative();
		common_device->SetPath("onigiri:/common/", nullptr, true);
		common_device->Mount("common:/");

		common_device_crc = new fiDeviceRelative();
		common_device_crc->SetPath("onigiri:/common/", nullptr, true);
		common_device_crc->Mount("commoncrc:/");

		dlc_device = new fiDeviceRelative();
		dlc_device->SetPath("onigiri:/dlcpacks/", nullptr, true);
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

	STATICALLY_INITIALIZE(custom_device)([]()
	{
		services::logger::info("applying custom device hooks");

		custom_device::open_archive = utils::detour(hook::get_pattern("48 81 EC 48 01 00 00 45 89 CD", -0x0C), custom_device::open_archive_hook);

		{
			auto location = hook::pattern("66 C7 44 58 02 00 00").count(1).get(0).get<void>(0xE);
			custom_device::initial_mount = utils::call(location, custom_device::initial_mount_hook);
		}

		/*{
			void* location = hook::pattern("48 8D ? ? ? ? ? B2 01 41 B8 FF FF FF FF 45 31 C9").count(1).get(0).get<void>(43);
			custom_device::load_level_metas = utils::call(location, custom_device::load_level_metas_hook);
		}*/

		// increase non-DLC fiDevice mount limit
		{
			auto location = hook::get_pattern<int>("C7 05 ? ? ? ? 64 00 00 00 C7 05 ? ? ? ? 00 01 00 00", 6);
			hook::put<int>(location, *location * 15); // '1500' mount limit now, instead of '500'
		}

		// don't sort update:/ relative devices before ours
		hook::nop(hook::pattern("C6 80 00 01 00 00 00 48 83 C4 20").count(1).get(0).get<void>(-12), 5);

		// don't crash (forget to call rage::fiDevice::Unmount) on failed DLC text reads
		// hmm..
		/*{
			auto location = hook::get_pattern("41 8B D6 E9 7C 02 00 00", 4);
			*(int*)location -= 0x12;
		}*/
	});

}
