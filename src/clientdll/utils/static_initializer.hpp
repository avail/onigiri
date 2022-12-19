#pragma once

#define STATICALLY_INITIALIZE_CLASSLESS ::onigiri::utils::static_initializer _
#define STATICALLY_INITIALIZE(T) ::onigiri::utils::static_initializer T::initializer_
#define STATICALLY_INITIALIZED(T) static ::onigiri::utils::static_initializer initializer_

namespace onigiri::utils
{
	class static_initializer final
	{
	public:
		using function_type = void(*)();

		static void init();

		static_initializer(static_initializer::function_type function, std::uint32_t priority);

		__forceinline static_initializer(static_initializer::function_type function) : static_initializer::static_initializer(function, 0)
		{

		}

		__forceinline void run()
		{
			this->function_();
		}

	private:
		DISABLE_COPYING(static_initializer);

		static_initializer::function_type function_;
		std::uint32_t priority_;
		utils::static_initializer* next_;

		static inline utils::static_initializer* initializers_;
	};
}
