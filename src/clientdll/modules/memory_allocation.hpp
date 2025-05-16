#pragma once

namespace onigiri
{
	class memory_allocation final
	{
	private:
		STATICALLY_INITIALIZED(memory_allocation);

	private:
		static void set_game_physical_budget(uint64_t budget);

		static inline uint64_t* vram_location;

		// use 1000 for one so we catch 'hardware reserved' memory as well
		static inline uint64_t gb = 1000 * 1024 * 1024;
		static inline float budget_multiplier = 1.29f; // 1.15f
	};
}
