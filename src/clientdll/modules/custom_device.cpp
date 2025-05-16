#include <stdinc.hpp>

#include <gta/fiPackfile.hpp>
#include <gta/fiDeviceRelative.hpp>

#include <utils/static_initializer.hpp>

#include <modules/custom_device.hpp>

namespace onigiri::modules
{
	static hook::cdecl_stub<rage::fiDevice* (const char*, bool)> GetDeviceHook([]()
	{
		return hook::get_pattern("48 81 EC 88 05 00 00 89 D6 49 89 CE", -12);
	});

	char custom_device::open_archive_hook(rage::fiPackfile* a1, const char* path, char a3, int32_t type, __int64 a5)
	{
		auto device = GetDeviceHook(path, true);

		if (device && custom_device::is_custom_device((rage::fiDeviceRelative*)device))
		{
			if (device->GetAttributes(path) & FILE_ATTRIBUTE_DIRECTORY)
			{
				onigiri::services::logger::debug("archive {} is a directory~", path);
				type = 2;
			}

			onigiri::services::logger::debug("opening archive {} {} {}", path, a3, type);
		}

		onigiri::services::logger::debug("opening archive {} {} {} {:X}", path, a3, type, (uint64_t)a5);
		return custom_device::open_archive(a1, path, a3, type, a5);
	}

	void custom_device::initial_mount_hook()
	{
		custom_device::initial_mount();

		std::filesystem::path cwd = std::filesystem::current_path() / "onigiri";
		cwd.make_preferred();

		onigiri::services::logger::info("mods path: {}", cwd.string());

		auto* root_device = new rage::fiDeviceRelative();
		root_device->Init(cwd.string().c_str(), true, nullptr);
		root_device->MountAs("onigiri:/");

		onigiri::services::logger::info("root device mounted~");

		platform_device = new rage::fiDeviceRelative();
		platform_device->Init("onigiri:/platform/", true, nullptr);
		platform_device->MountAs("platform:/");

		platform_device_crc = new rage::fiDeviceRelative();
		platform_device_crc->Init("onigiri:/platform/", true, nullptr);
		platform_device_crc->MountAs("platformcrc:/");

		platform2_device = new rage::fiDeviceRelative();
		platform2_device->Init("onigiri:/platform2/", true, nullptr);
		platform2_device->MountAs("platform2:/");

		platform2_device_crc = new rage::fiDeviceRelative();
		platform2_device_crc->Init("onigiri:/platform2/", true, nullptr);
		platform2_device_crc->MountAs("platform2crc:/");

		common_device = new rage::fiDeviceRelative();
		common_device->Init("onigiri:/common/", true, nullptr);
		common_device->MountAs("common:/");

		common_device_crc = new rage::fiDeviceRelative();
		common_device_crc->Init("onigiri:/common/", true, nullptr);
		common_device_crc->MountAs("commoncrc:/");

		dlc_device = new rage::fiDeviceRelative();
		dlc_device->Init("onigiri:/dlcpacks/", true, nullptr);
		dlc_device->MountAs("dlcpacks:/");
	}

	void custom_device::load_level_metas_hook(void* data_file_manager, const char* name, bool enabled)
	{
		{
			auto meh = std::filesystem::current_path() / "onigiri" / "before_level.xml";
			onigiri::services::logger::info("loading before-level~ ({})", meh.string());
			custom_device::load_level_metas(data_file_manager, &meh.string()[0], enabled);
		}

		onigiri::services::logger::info("loading level~ ({})", name);
		custom_device::load_level_metas(data_file_manager, name, enabled);

		{
			auto meh = std::filesystem::current_path() / "onigiri" / "after_level.xml";
			onigiri::services::logger::info("loading after-level~ ({})", meh.string());
			custom_device::load_level_metas(data_file_manager, &meh.string()[0], enabled);
		}
	}

	STATICALLY_INITIALIZE(custom_device)([]()
	{
		onigiri::services::logger::info("applying custom device hooks~");

		//custom_device::open_archive = utils::detour(hook::get_pattern("48 81 EC 48 01 00 00 45 89 CD", -0x0C), custom_device::open_archive_hook);

		{
			auto location = hook::pattern("66 C7 44 58 02 00 00").count(1).get(0).get<void>(0xE);
			custom_device::initial_mount = utils::call(location, custom_device::initial_mount_hook);
		}

		{
			void* location = hook::pattern("48 8D ? ? ? ? ? B2 01 41 B8 FF FF FF FF 45 31 C9").count(1).get(0).get<void>(43);
			custom_device::load_level_metas = utils::call(location, custom_device::load_level_metas_hook);
		}

		// increase non-DLC fiDevice mount limit
		{
			auto location = hook::get_pattern<int>("C7 05 ? ? ? ? 64 00 00 00 C7 05 ? ? ? ? 00 01 00 00", 6);
			hook::put<int>(location, *location * 15); // '1500' mount limit now, instead of '500'
		}

		// don't sort update:/ relative devices before ours
		hook::nop(hook::pattern("C6 80 00 01 00 00 00 48 83 C4 20").count(1).get(0).get<void>(-12), 5);
	});

}
