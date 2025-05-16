#include <stdinc.hpp>

#define DEBUG_LOADS 0

HANDLE OpenBulkHook(void* device, const char* path, char a3)
{

	HANDLE FileW;
	WCHAR WideCharStr[256];

	a3 = GENERIC_READ;

	MultiByteToWideChar(0xFDE9u, 0, path, -1, WideCharStr, 256);

	FileW = CreateFileW((L"mods/" + std::wstring(WideCharStr)).c_str(), 0x80000000, 1u, 0, 3, 0x80, 0);

	if (FileW == (HANDLE)-1)
	{
		FileW = CreateFileW(WideCharStr, 0x80000000, 1u, 0, 3, 0x80, 0);
	}
#if DEBUG_LOADS
	else
	{
		onigiri::services::logger::info("[{}] Found mods/{}", __FUNCTION__, path);
	}
#endif

	return FileW;
}

FILETIME GetFileTimeHook(void* device, const char* path)
{
	//onigiri::services::logger::info("[{}] {}", __FUNCTION__, path);

	WCHAR WideCharStr[256];
	WIN32_FILE_ATTRIBUTE_DATA FileInformation;

	MultiByteToWideChar(0xFDE9u, 0, path, -1, WideCharStr, 256);

	if (GetFileAttributesExW((L"mods/" + std::wstring(WideCharStr)).c_str(), GetFileExInfoStandard, &FileInformation))
	{
#if DEBUG_LOADS
		onigiri::services::logger::info("[{}] Found mods/{}", __FUNCTION__, path);
#endif
		return FileInformation.ftLastWriteTime;
	}
	else if (GetFileAttributesExW(WideCharStr, GetFileExInfoStandard, &FileInformation))
	{
		return FileInformation.ftLastWriteTime;
	}

	return (FILETIME)0;
}

uint64_t GetFileSizeHook(void* device, const char* path)
{
	//onigiri::services::logger::info("[{}] {}", __FUNCTION__, path);

	WCHAR WideCharStr[256];
	WIN32_FILE_ATTRIBUTE_DATA FileInformation;

	MultiByteToWideChar(0xFDE9u, 0, path, -1, WideCharStr, 256);

	if (GetFileAttributesExW((L"mods/" + std::wstring(WideCharStr)).c_str(), GetFileExInfoStandard, &FileInformation))
	{
#if DEBUG_LOADS
		onigiri::services::logger::info("[{}] Found mods/{}", __FUNCTION__, path);
#endif
		return FileInformation.nFileSizeLow | (static_cast<size_t>(FileInformation.nFileSizeHigh) << 32);
	}
	else if (GetFileAttributesExW(WideCharStr, GetFileExInfoStandard, &FileInformation))
	{
		return FileInformation.nFileSizeLow | (static_cast<size_t>(FileInformation.nFileSizeHigh) << 32);
	}

	return 0;
}

uint64_t GetAttributesHook(void* device, const char* path)
{
	WCHAR WideCharStr[256];
	DWORD FileAttributesW;

	MultiByteToWideChar(0xFDE9u, 0, path, -1, WideCharStr, 256);

	FileAttributesW = GetFileAttributesW((L"mods/" + std::wstring(WideCharStr)).c_str());

	if (FileAttributesW == -1)
	{
		FileAttributesW = GetFileAttributesW(WideCharStr);
	}
	else
	{
#if DEBUG_LOADS
		onigiri::services::logger::info("[{}] Found mods/{}", __FUNCTION__, path);
#endif
	}

	return FileAttributesW;
}

static onigiri::utils::static_initializer _([]()
{
	return;

	hook::jump(hook::get_pattern("48 81 EC 98 04 00 00 44 89 C3", -5), OpenBulkHook);

	auto location = hook::pattern("48 81 EC 70 02 00 00 66 C7 44 24 60 00 00");
	hook::jump(location.count(2).get(1).get<void*>(-3), GetFileTimeHook);
	hook::jump(location.count(2).get(0).get<void*>(-3), GetFileSizeHook);

	hook::jump(hook::get_pattern("48 81 EC 98 04 00 00 66 C7 44 24 38 00 00", -5), GetAttributesHook);
});
