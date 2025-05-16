#include <stdinc.hpp>

#include <gta/fiDeviceRelative.hpp>

static hook::cdecl_stub<bool* (rage::fiDeviceRelative*, const char*)> mount_wrap([]()
{
	return hook::get_pattern("44 0F B6 81 14 01 00 00", -0x1B);
});

static hook::cdecl_stub<void(void*, const char*, bool, rage::fiDevice*)> set_path([]()
{
	return hook::get_pattern("41 B8 00 01 00 00 E8 ? ? ? ? 88 9E", -0x36);
});

rage::fiDeviceRelative::fiDeviceRelative()
{
	auto result = hook::pattern("B9 18 01 00 00 E8 ? ? ? ? 48 89 C6 48 8D").count(1).get(0).get<uint32_t>(16);
	uintptr_t endOffset = ((uintptr_t)result) + 4;
	auto g_vTable_fiDeviceRelative = endOffset + *result;

	*(uintptr_t*)this = g_vTable_fiDeviceRelative;

	this->m_pad[256] = '\0';
}

void rage::fiDeviceRelative::Init(const char* relativeTo, bool allowRoot, rage::fiDevice* baseDevice)
{
	return set_path(this, relativeTo, allowRoot, baseDevice);
}

bool rage::fiDeviceRelative::MountAs(const char* mountPoint) // mountAs
{
	return mount_wrap(this, mountPoint);
}
