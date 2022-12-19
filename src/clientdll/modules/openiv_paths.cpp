#include <stdinc.hpp>

#include <utils/Hooking.Invoke.h>
#include <utils/Hooking.h>
#include <utils/Hooking.IAT.h>

HANDLE OpenBulkHook(void* device, const char* path, __int64* a3)
{
	HANDLE FileW;
	WCHAR WideCharStr[256];

	*a3 = 0;

	MultiByteToWideChar(0xFDE9u, 0, path, -1, WideCharStr, 256);

	FileW = CreateFileW((L"onigiri_mods/" + std::wstring(WideCharStr)).c_str(), 0x80000000, 1u, 0, 3, 0x80, 0);

	if (FileW == (HANDLE)-1)
	{
		FileW = CreateFileW(WideCharStr, 0x80000000, 1u, 0, 3, 0x80, 0);
	}
	else
	{
		onigiri::services::logger::info("Found onigiri_mods/%s", path);
	}

	return FileW;
}

FILETIME GetFileTimeHook(void* device, const char* path)
{
	WCHAR WideCharStr[256];
	WIN32_FILE_ATTRIBUTE_DATA FileInformation;

	MultiByteToWideChar(0xFDE9u, 0, path, -1, WideCharStr, 256);

	if (GetFileAttributesExW((L"onigiri_mods/" + std::wstring(WideCharStr)).c_str(), GetFileExInfoStandard, &FileInformation))
	{
		onigiri::services::logger::info("Found onigiri_mods/%s", path);
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
	WCHAR WideCharStr[256];
	WIN32_FILE_ATTRIBUTE_DATA FileInformation;

	MultiByteToWideChar(0xFDE9u, 0, path, -1, WideCharStr, 256);

	if (GetFileAttributesExW((L"onigiri_mods/" + std::wstring(WideCharStr)).c_str(), GetFileExInfoStandard, &FileInformation))
	{
		onigiri::services::logger::info("[%s] Found onigiri_mods/%s", __FUNCTION__, path);
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

	FileAttributesW = GetFileAttributesW((L"onigiri_mods/" + std::wstring(WideCharStr)).c_str());

	if (FileAttributesW == -1)
	{
		FileAttributesW = GetFileAttributesW(WideCharStr);
	}
	else
	{
		onigiri::services::logger::info("Found onigiri_mods/%s", path);
	}

	return FileAttributesW;
}

static HookFunction _([]()
{
	return;
	hook::jump(hook::get_pattern("40 53 48 81 EC ? ? ? ? 49 8B D8 4C 8B C2 48 8D 4C 24 ? BA ? ? ? ? E8 ? ? ? ? 48 83 64 24"), OpenBulkHook);
	hook::jump(hook::get_pattern("48 81 EC ? ? ? ? 4C 8B C2 48 8D 4C 24 ? BA ? ? ? ? E8 ? ? ? ? 4C 8D 44 24 ? 33 D2 48 8B C8 FF 15 ? ? ? ? 85 C0 75 04 33 C0 EB 0F 8B 44 24 38 8B 4C 24 34 48 C1 E0 20 48 0B C1 48 81 C4"), GetFileTimeHook);
	hook::jump(hook::get_pattern("48 81 EC ? ? ? ? 4C 8B C2 48 8D 4C 24 ? BA ? ? ? ? E8 ? ? ? ? 4C 8D 44 24 ? 33 D2 48 8B C8 FF 15 ? ? ? ? 85 C0 75 04 33 C0 EB 0F 8B 44 24 3C 8B 4C 24 40 48 C1 E0 20 48 0B C1 48 81 C4"), GetFileSizeHook);
	hook::jump(hook::get_pattern("48 89 5C 24 ? 57 48 81 EC ? ? ? ? 4C 8B C2 48 8D 4C 24 ? BA ? ? ? ? E8 ? ? ? ? 48 8B C8 FF 15 ? ? ? ? 83 CF FF 8B D8 3B C7 74 0F 48 8D 4C 24"), GetAttributesHook);
});
