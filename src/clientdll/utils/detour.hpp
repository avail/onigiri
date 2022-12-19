#pragma once

namespace onigiri::utils
{
	template <typename F> class function final
	{
	public:
		__forceinline function() : function<F>::function(nullptr)
		{

		}

		__forceinline function(F* ptr) : function_(ptr)
		{

		}

		__forceinline function(const std::uint32_t address) : function<F>::function(reinterpret_cast<F*>(address))
		{

		}

		template <typename... Args> __forceinline auto operator()(Args&& ...args) const -> decltype(auto)
		{
			return this->function_(args...);
		}

	private:
		F* function_;
	};

	class detour final
	{
	public:
		static inline void init()
		{
			MH_Initialize();
		}

		static inline void shutdown()
		{
			MH_DisableHook(MH_ALL_HOOKS);
			MH_Uninitialize();
		}

		template <typename F> static __forceinline utils::function<F> make(void* address, F* function)
		{
			auto result = utils::function<F>();

			MH_CreateHook(address, reinterpret_cast<void*>(function), reinterpret_cast<void**>(&result));
			MH_EnableHook(MH_ALL_HOOKS);

			return result;
		}
	};
}
