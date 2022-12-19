#include <stdinc.hpp>

#include <injector.hpp>

void log(const char* log)
{
	printf(log);
}

int main(int argc, char* argv[])
{
	log("waiting for gta5.exe\n");

	// xdddddd
	system("\"C:\\Program Files\\Rockstar Games\\Grand Theft Auto V\\PlayGTAV.exe\"");

	auto pid = injector::wait_for_process("GTA5.exe");

	auto libPath = std::filesystem::current_path() / "onigiri.dll";

	log(std::format("got gta5 process: {}\n", pid).c_str());

	auto res = injector::inject_dll(pid, libPath);

	if (res)
	{
		log("injected!\n");
	}
	else
	{
		log("not so injected\n");
	}
}
