#include <stdinc.hpp>

#include <modules/memory_allocation.hpp>

namespace onigiri
{
	void memory_allocation::set_game_physical_budget(uint64_t budget)
	{
		// externally, there are 4 texture settings:
		// 0: 'normal'
		// 1: 'high'
		// 2: 'very high'
		// 3: no override
		//
		// however, these are implemented by means of an internal flag which is the amount of mips to cut off, as follows:
		// 0: 'low' (non-existent setting, cut off 2 mips)
		// 1: 'normal' (cut off 1 mip)
		// 2: 'high' (cut off no mips)
		// x: 'very high' (cut off no mips, and allow +hi TXDs)
		//
		// the issue is R* seems to have thought in a few cases that the internal texture setting flag mapped to normal, high, very high,
		// and not unused, normal, high/very high, where 'very high' is just 'high' with the +hi loading flag enabled.
		//
		// instead, we'll just keep high and very high the same, but move the 'low' flag to 'normal' and lower the 'low' size some more in case
		// anyone feels like enabling the real 'low'
		for (int i = 0; i < 80; i += 4)
		{
			vram_location[i + 3] = static_cast<uint64_t>(budget * budget_multiplier);
			vram_location[i + 2] = static_cast<uint64_t>(budget * budget_multiplier);
			vram_location[i + 1] = static_cast<uint64_t>((budget * budget_multiplier) / 1.5f);
			vram_location[i] = static_cast<uint64_t>((budget * budget_multiplier) / 2.0f);
		}
	}

	STATICALLY_INITIALIZE(memory_allocation)([]()
	{
		// don't initialize some odd timer watchdog thread that crashes a *lot* of people
		hook::return_function(hook::pattern("41 B8 ? ? ? ? 41 B9 ? ? ? ? E8 ? ? ? ? 48 89 05 ? ? ? ? 48 83 C4 ? C3").count(1).get(0).get<void>(-0x29));

		// extend grcResourceCache pool a bit
		// #GEN9: missing?
		/*{
			auto location = hook::get_pattern<char>("BA 00 00 05 00 48 8B C8 44 88");
			hook::put<uint32_t>(location + 1, 0xA0000);
			hook::put<uint32_t>(location + 23, 0xA001B);
		}*/

		MEMORYSTATUSEX msex = { 0 };
		msex.dwLength = sizeof(msex);

		GlobalMemoryStatusEx(&msex);

		uint32_t allocator_reservation = 0;
		bool should_allocate = true;

		if (msex.ullTotalPhys >= 32 * gb)
		{
			allocator_reservation = 0xE0000000; // 3.5gb
		}
		if (msex.ullTotalPhys >= 24 * gb)
		{
			allocator_reservation = 0xC0000000; // 3gb
		}
		else if (msex.ullTotalPhys >= 16 * gb)
		{
			allocator_reservation = 0x80000000; // 2gb
		}
		else if (msex.ullTotalPhys >= 12 * gb)
		{
			allocator_reservation = 0x60000000; // 1.5gb
		}
		else if (msex.ullTotalPhys >= 8 * gb)
		{
			allocator_reservation = 0x40000000; // 1gb
		}
		else
		{
			should_allocate = false;
		}

		// #GEN9: need vram_location
		//vram_location = hook::get_address<uint64_t*>(hook::get_pattern("4C 63 C0 48 8D 05 ? ? ? ? 48 8D 14", 6));
		//memory_allocation::set_game_physical_budget(3 * gb);

		if (should_allocate)
		{
			// increase allocator amount
			{
				auto location = hook::get_pattern("BA ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 48 8D 0D ? ? ? ? E8", 7);
				hook::put<uint32_t>(location, allocator_reservation);
			}
		}
	});
}
