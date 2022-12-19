#pragma once

#include <string_view>
#include <filesystem>

namespace onigiri::utils
{
	class storage final
	{
	public:
		static inline void init()
		{
			std::filesystem::create_directories(get_base_path());
		}

		static inline std::string get_relative_storage_path(const std::string_view& relative)
		{
			auto path = get_base_path();
			path += "\\";
			path += relative;

			return path;
		}

	private:
		DISABLE_COPYING(storage);

		static inline std::string get_base_path()
		{
			std::string path = std::getenv("appdata");
			path += "\\REKT\\onigiri";
			
			return path;
		}
	};
}
