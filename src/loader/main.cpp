#include <stdinc.hpp>

#include <injector.hpp>

void log(const char* log)
{
	printf(log);
}

int main(int argc, char* argv[])
{
	log("launching gta5.exe\n");

	/*
	* GTA5_Enhanced.exe 
	*/
	auto program = "";
	auto args = "-enableCrashpad -scCommerceProvider=4 @commandline.txt -fromRGL -scDiscordClientId=1329870933695135785 -rglLanguage=en-US -nobattleye";
	system(std::format("\"{}\" {}", program, args).c_str());

	// xdddddd
	/*system("\"C:\\Program Files\\Rockstar Games\\Grand Theft Auto V\\PlayGTAV.exe\"");

	auto pid = injector::wait_for_process("GTA5.exe");

	auto lib_path = std::filesystem::current_path() / "onigiri.dll";

	log(std::format("got gta5 process: {}\n", pid).c_str());

	auto res = injector::inject_dll(pid, lib_path);

	if (res)
	{
		log("injected!\n");
	}
	else
	{
		log("not so injected\n");
	}*/
}
