#pragma once

#include <string_view>
#include <filesystem>

namespace onigiri::utils
{
	class storage final
	{
	public:
		static void init();

	public:
		static inline bool SkipIntro = true;
		static inline bool SkipLegal = true;
		static inline bool SkipSplash = true;

		static inline std::string get_relative_storage_path(const std::string_view& relative)
		{
			auto path = get_base_path();
			path += "\\";
			path += relative;

			return path;
		}

		static inline std::string get_game_path(const std::string& path)
		{
			static std::string gamePath;

			if (gamePath.empty())
			{
				char exeName[512];
				GetModuleFileNameA(GetModuleHandle(NULL), exeName, sizeof(exeName));

				char* exeBaseName = strrchr(exeName, '\\');
				exeBaseName[0] = L'\0';

				gamePath = exeName;
				gamePath += "\\";

				GetFullPathNameA(gamePath.c_str(), sizeof(exeName), exeName, nullptr);

				gamePath = exeName;
				gamePath += "\\";
			}

			return gamePath + path;
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
