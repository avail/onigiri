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

	template <typename F> inline utils::function<F> detour(void* address, F* function)
	{
		auto result = utils::function<F>();

		if (::MH_CreateHook(reinterpret_cast<void*>(address), reinterpret_cast<void*>(function), reinterpret_cast<void**>(&result)) != ::MH_OK)
		{
			//NFSCO_ASSERT(false);
		}

		if (::MH_EnableHook(MH_ALL_HOOKS) != ::MH_OK)
		{
			//NFSCO_ASSERT(false);
		}

		return result;
	}

	template <typename F> inline utils::function<F> call(void* address, F* function)
	{
		auto result = utils::function<F>();

		DWORD oldProtect;
		VirtualProtect(address, 16, PAGE_EXECUTE_READWRITE, &oldProtect);

		hook::set_call(&result, address);
		hook::call(address, function);

		return result;
	}

	template <typename F> inline utils::function<F> iat(const wchar_t* module_name, const char* function_name, F* function)
	{
		auto result = utils::function<F>(nullptr);

		if (MH_CreateHookApi(module_name, function_name, reinterpret_cast<void*>(function), reinterpret_cast<void**>(&result)) != ::MH_OK)
		{
			//NFSCO_ASSERT(false);
		}

		if (MH_EnableHook(MH_ALL_HOOKS) != ::MH_OK)
		{
			//NFSCO_ASSERT(false);
		}

		return result;
	}

	inline void init()
	{
		MH_Initialize();
	}

	inline void shutdown()
	{
		MH_Uninitialize();
	}
}
