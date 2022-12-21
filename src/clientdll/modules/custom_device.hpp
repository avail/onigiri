#pragma once

namespace onigiri::modules
{
	class custom_device final
	{
	private:
		STATICALLY_INITIALIZED(custom_device);

		static char open_archive_hook(rage::fiPackfile* a1, const char* path, char a3, int32_t type, __int64 a5);
		static inline utils::function<decltype(open_archive_hook)> open_archive;

		static void initial_mount_hook();
		static inline utils::function<decltype(initial_mount_hook)> initial_mount;

		static void load_level_metas_hook(void* data_file_manager, const char* name, bool enabled);
		static inline utils::function<decltype(load_level_metas_hook)> load_level_metas;

		static bool set_thread_count_hook(void* param, int* value);
		static inline utils::function<decltype(set_thread_count_hook)> set_thread_count;

		static inline rage::fiDeviceLocal* platform_device;
		static inline rage::fiDeviceLocal* platform_device_crc;
		static inline rage::fiDeviceLocal* platform2_device;
		static inline rage::fiDeviceLocal* platform2_device_crc;
		static inline rage::fiDeviceLocal* common_device;
		static inline rage::fiDeviceLocal* common_device_crc;
		static inline rage::fiDeviceLocal* dlc_device;

	private:
		static inline bool is_custom_device(rage::fiDeviceLocal* dev)
		{
			if (dev == platform_device ||
				dev == platform_device_crc ||
				dev == platform2_device ||
				dev == platform2_device_crc ||
				dev == common_device ||
				dev == common_device_crc ||
				dev == dlc_device)
			{
				return true;
			}

			return false;
		}

	};
}
