#pragma once

namespace onigiri::modules
{
	class reflections final
	{
	private:
		STATICALLY_INITIALIZED(reflections);

		static int get_reflection_resolution(int screen_width, int screen_height, int reflection_quality);

		static void* reflection_render_hook(void* this_);
		static inline utils::function<decltype(reflection_render_hook)> reflection_render;
	};
}
