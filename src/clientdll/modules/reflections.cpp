#include <stdinc.hpp>

#include <modules/reflections.hpp>

#include <utils/Hooking.h>
#include <utils/Hooking.Patterns.h>
#include <utils/static_initializer.hpp>
#include <MinHook.h>

#include <gta/CRenderPhase.hpp>

namespace onigiri::modules
{
	int reflections::get_reflection_resolution(int screen_width, int screen_height, int reflection_quality)
	{
		int resHeight{ 32 };
		int resWidth{ 0 };

		if (reflection_quality <= 0)
		{
			resWidth = 32;
		}
		else
		{
			if (reflection_quality > 3)
			{
				reflection_quality = 3;
			}

			resWidth = screen_width / (1 << (3 - reflection_quality));
			if (resWidth < 1)
			{
				resWidth = 1;
			}

		}
		if (reflection_quality > 0)
		{
			if (reflection_quality > 3)
			{
				reflection_quality = 3;
			}

			resHeight = screen_height / (1 << (3 - reflection_quality));

			if (resHeight < 1)
			{
				resHeight = 1;
			}

		}

		int highestBitPos{ 0 };
		const float fVar{ (float)(resWidth * resHeight) };

		const int root = _mm_sqrt_ps(_mm_load_ps(&fVar)).m128_f32[0];

		for (int i = root; i; i >>= 1)
		{
			highestBitPos++;
		}

		const int result = (unsigned int)(1LL << (highestBitPos - 1)) * 2;
		return result;
	}

	static void*(*g_origCRenderPhaseReflectionRender)(void* this_);
	static void* CRenderPhaseReflectionRenderWrap(void* this_)
	{
		static float* g_unkFloat1 = []()
		{
			return hook::get_address<float*>(hook::get_pattern("44 21 35 ? ? ? ? 44 21 35 ? ? ? ? F3 0F 10 05"), 3, 7);
		}();

		static float* g_lodRadius = []()
		{
			return hook::get_address<float*>(hook::get_pattern("44 21 35 ? ? ? ? F3 0F 10 05 ? ? ? ? F3 0F 11 05 ? ? ? ? 4C 8D 9C 24 ? ? ? ? 49 8B 5B 10"), 3, 7);
		}();

		if (RenderState::m_enableReflections)
		{
			void* result = g_origCRenderPhaseReflectionRender(this_);
			*g_unkFloat1 = RenderState::m_unkFloat1;
			*g_lodRadius = RenderState::m_lodRadius;

			return result;
		}

		return nullptr;
	}

	STATICALLY_INITIALIZE(reflections)([]()
	{
		//return;

		onigiri::services::logger::info("initializing reflection quality module~");
		hook::call(hook::get_pattern<void>("E8 ? ? ? ? 8B D0 44 8B F0 B8"), reflections::get_reflection_resolution);

		auto orig = hook::pattern("48 8B C4 48 89 58 08 48 89 70 10 48 89 78 18 4C 89 70 20 55 48 81 EC ? ? ? ? 0F 29 70 E8").get_first<void*>();
		MH_CreateHook(orig, CRenderPhaseReflectionRenderWrap, (void**)&g_origCRenderPhaseReflectionRender);
		MH_EnableHook(orig);

		static uintptr_t functionArgs = (uintptr_t)hook::get_pattern<void*>("BA ? ? ? ? 41 B8 ? ? ? ? 48 89 44 24 ? E8 ? ? ? ? 4C 8D 45 28");

		static int* entityIndex1{ (int*)(functionArgs + 1) };
		static int* entityIndex2{ (int*)(functionArgs + 7) };
		static BYTE* entityIndex3{ (BYTE*)(functionArgs - 13) };

		*entityIndex1 = 3;
	});
}
