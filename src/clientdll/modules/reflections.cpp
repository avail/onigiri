#include <stdinc.hpp>

#include <modules/reflections.hpp>

#if 0
#include <intrin.h>

#include <gta/CRenderPhase.hpp>

namespace onigiri::modules
{
	int reflections::get_reflection_resolution(int screen_width, int screen_height, int reflection_quality)
	{
		int res_height{ 32 };
		int res_width{ 0 };

		if (reflection_quality <= 0)
		{
			res_width = 32;
		}
		else
		{
			if (reflection_quality > 3)
			{
				reflection_quality = 3;
			}

			res_width = screen_width / (1 << (3 - reflection_quality));
			if (res_width < 1)
			{
				res_width = 1;
			}

		}
		if (reflection_quality > 0)
		{
			if (reflection_quality > 3)
			{
				reflection_quality = 3;
			}

			res_height = screen_height / (1 << (3 - reflection_quality));

			if (res_height < 1)
			{
				res_height = 1;
			}

		}

		int highest_bit{ 0 };
		const float fVar{ (float)(res_width * res_height) };

		const auto root = static_cast<std::int32_t>(_mm_sqrt_ps(_mm_load_ps(&fVar)).m128_f32[0]);

		for (int i = root; i; i >>= 1)
		{
			highest_bit++;
		}

		const int result = (unsigned int)(1LL << (highest_bit - 1)) * 2;
		return result;
	}

	void* reflections::reflection_render_hook(void* this_)
	{
		static float* unk_float_1 = []()
		{
			return hook::get_address<float*>(hook::get_pattern("44 21 35 ? ? ? ? 44 21 35 ? ? ? ? F3 0F 10 05"), 3, 7);
		}();

		static float* lod_radius = []()
		{
			return hook::get_address<float*>(hook::get_pattern("44 21 35 ? ? ? ? F3 0F 10 05 ? ? ? ? F3 0F 11 05 ? ? ? ? 4C 8D 9C 24 ? ? ? ? 49 8B 5B 10"), 3, 7);
		}();

		if (RenderState::m_enableReflections)
		{
			void* result = reflection_render(this_);
			*unk_float_1 = RenderState::m_unkFloat1;
			*lod_radius = RenderState::m_lodRadius;

			return result;
		}

		return nullptr;
	}

	STATICALLY_INITIALIZE(reflections)([]()
	{
		return;

		onigiri::services::logger::info("initializing reflection quality module~");
		hook::call(hook::get_pattern<void>("E8 ? ? ? ? 8B D0 44 8B F0 B8"), reflections::get_reflection_resolution);

		reflection_render = utils::detour(hook::pattern("48 8B C4 48 89 58 08 48 89 70 10 48 89 78 18 4C 89 70 20 55 48 81 EC ? ? ? ? 0F 29 70 E8").get_first<void*>(), reflections::reflection_render_hook);

		static uintptr_t function_args = (uintptr_t)hook::get_pattern<void*>("BA ? ? ? ? 41 B8 ? ? ? ? 48 89 44 24 ? E8 ? ? ? ? 4C 8D 45 28");

		static int* entity_index_1{ (int*)(function_args + 1) };
		static int* entity_index_2{ (int*)(function_args + 7) };
		static BYTE* entity_index_3{ (BYTE*)(function_args - 13) };

		*entity_index_1 = 3;
	});
}
#endif
