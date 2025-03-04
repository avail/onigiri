#include <stdinc.hpp>

#include <utils/static_initializer.hpp>

#if 0
static hook::cdecl_stub<uint32_t* (uint32_t*, const char*, bool, const char*, bool)> func([]()
{
	return hook::get_pattern("B2 01 48 8B CD 45 8A E0 4D 0F 45 F9 E8", -0x25);
});

uint32_t* register_raw_streaming_file(uint32_t* fileId, const char* fileName, bool unkTrue, const char* registerAs, bool errorIfFailed)
{
	return func(fileId, fileName, unkTrue, registerAs, errorIfFailed);
}

void present_hook(int syncInterval, int flags);

void set_hook()
{
	hook::put(hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? 48 85 C0 74 0C 8B 4D 50 8B", 3)), present_hook);
}

void unset_hook()
{
	hook::put(hook::get_address<void*>(hook::get_pattern("48 8B 05 ? ? ? ? 48 85 C0 74 0C 8B 4D 50 8B", 3)), (uint64_t)0);
}

void present_hook(int syncInterval, int flags)
{
	auto reg = [](const std::string& path, const std::string& name)
	{
		uint32_t file_id = -1;
		register_raw_streaming_file(&file_id, path.c_str(), true, name.c_str(), false);

		if (file_id == -1)
		{
			onigiri::services::logger::info("Failed registering asset %s", name);
		}
		else
		{
			onigiri::services::logger::info("Successfully registered asset %s", name);
		}
	};

	reg("H:\\Data\\onigiri\\assets\\banshee.ytd", "banshee.ytd");
	reg("H:\\Data\\onigiri\\assets\\banshee.yft", "banshee.yft");
	reg("H:\\Data\\onigiri\\assets\\banshee_hi.yft", "banshee_hi.yft");

	unset_hook();
}

static onigiri::utils::static_initializer _([]()
{
	return;

	onigiri::services::logger::info("streaming init~");

	set_hook();
});
#endif
