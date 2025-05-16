#include <stdinc.hpp>

#include <utils/storage.hpp>

#include <ini_parser.hpp>

namespace onigiri::utils
{
	auto wb(bool value) -> std::string
	{
		return value ? "true" : "false";
	};

	auto rb(const std::string& value) -> bool
	{
		return value[0] == 't' ? true : false;
	};

	void storage::init()
	{
		std::filesystem::create_directories(get_base_path());
		auto filePath = get_game_path("onigiri.ini");

		linb::ini ini;
		if (ini.read_file(filePath.c_str()))
		{
			SkipIntro = rb(ini.get("PATCHES", "SkipIntro", "false"));
			SkipLegal = rb(ini.get("PATCHES", "SkipLegal", "true"));
			SkipSplash = rb(ini.get("PATCHES", "SkipSplash", "true"));

		}
		else
		{
			ini.set("PATCHES", "SkipIntro", wb(false));
			ini.set("PATCHES", "SkipLegal", wb(true));
			ini.set("PATCHES", "SkipSplash", wb(true));

			ini.write_file(filePath.c_str());
		}
	}
}
