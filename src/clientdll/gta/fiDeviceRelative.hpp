#pragma once

#include <gta/fiDevice.hpp>

namespace rage
{
	class fiDeviceRelative
	{
	private:
		char m_pad[272];
	public:
		fiDeviceRelative();

		void Init(const char* relativeTo, bool allowRoot, rage::fiDevice* baseDevice);
		bool MountAs(const char* mountPoint);
	};
}
