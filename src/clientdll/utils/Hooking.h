#pragma once
#include <stdint.h>
#include <type_traits>

/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 *
 * https://github.com/citizenfx/fivem/blob/master/code/client/shared/Hooking.h
 */

namespace hook
{
	template<typename TAddr, typename T>
	inline int get_rip_offset(TAddr address, size_t bytesInLine, T newAddress)
	{
		uintptr_t rip = (uintptr_t)address + bytesInLine;
		return (uintptr_t(newAddress)) - rip;
	}

	template<typename T>
	inline T get_call(T address)
	{
		intptr_t target = *(int*)((uintptr_t)address + 1);
		target += ((uintptr_t)address + 5);

		return (T)target;
	}

	template<typename T, typename TAddr>
	inline T get_address(TAddr address)
	{
		intptr_t target = *(int*)(uintptr_t)address;
		target += ((uintptr_t)address + 4);

		return (T)target;
	}

	template<typename T, typename TAddr>
	inline T get_address(TAddr address, size_t offsetTo4ByteAddr, size_t numBytesInLine)
	{
		intptr_t target = *(int*)((uintptr_t)address + offsetTo4ByteAddr);
		target += ((uintptr_t)(address) + numBytesInLine);

		return (T)target;
	}

	template<typename ValueType, typename AddressType>
	inline void put(AddressType address, ValueType value)
	{
		memcpy((void*)address, &value, sizeof(value));
	}

	template<typename AddressType>
	inline void nop(AddressType address, size_t length)
	{
		memset((void*)address, 0x90, length);
	}

	void* AllocateFunctionStub(void *origin, void *function, int type);

	template<typename T>
	struct get_func_ptr
	{
		static void* get(T func)
		{
			return (void*)func;
		}
	};

	template<int Register, typename T, typename AT>
	inline std::enable_if_t<(Register < 8 && Register >= 0)> jump_reg(AT address, T func)
	{
		LPVOID funcStub = AllocateFunctionStub((void*)GetModuleHandle(NULL), get_func_ptr<T>::get(func), Register);

		put<uint8_t>(address, 0xE9);
		put<int>((uintptr_t)address + 1, (intptr_t)funcStub - (intptr_t)address - 5);
	}

	template<typename T, typename AT>
	inline void jump(AT address, T func)
	{
		jump_reg<0>(address, func);
	}

	template<typename T, typename AT>
	inline void jump_rcx(AT address, T func)
	{
		jump_reg<1>(address, func);
	}

	template<int Register, typename T, typename AT>
	inline std::enable_if_t<(Register < 8 && Register >= 0)> call_reg(AT address, T func)
	{
		LPVOID funcStub = AllocateFunctionStub((void*)GetModuleHandle(NULL), get_func_ptr<T>::get(func), Register);

		put<uint8_t>(address, 0xE8);
		put<int>((uintptr_t)address + 1, (intptr_t)funcStub - (intptr_t)address - 5);
	}

	template<typename T, typename AT>
	inline void call(AT address, T func)
	{
		call_reg<0>(address, func);
	}
	
	template<typename T, typename AT>
	inline void call_rcx(AT address, T func)
	{
		call_reg<1>(address, func);
	}

	// gets the current executable TLS offset
	template<typename T = char*>
	T get_tls()
	{
		// ah, the irony in using TLS to get TLS
		static auto tlsIndex = ([]()
			{
				auto base = (char*)GetModuleHandle(NULL);
				auto moduleBase = (PIMAGE_DOS_HEADER)base;
				auto ntBase = (PIMAGE_NT_HEADERS)(base + moduleBase->e_lfanew);
				auto tlsBase = (PIMAGE_TLS_DIRECTORY)(base + ntBase->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);

				return reinterpret_cast<uint32_t*>(tlsBase->AddressOfIndex);
			})();

#if defined(_M_IX86)
			LPVOID* tlsBase = (LPVOID*)__readfsdword(0x2C);
#elif defined(_M_AMD64)
			LPVOID* tlsBase = (LPVOID*)__readgsqword(0x58);
#endif

			return (T)tlsBase[*tlsIndex];
	}

	inline uintptr_t get_member_internal(void* function)
	{
		return (uintptr_t)function;
	}

	template<typename T>
	inline uintptr_t get_member_old(T function)
	{
		return ((uintptr_t(*)(T))get_member_internal)(function);
	}

	template<typename TClass, typename TMember>
	inline uintptr_t get_member(TMember TClass::* function)
	{
		union member_cast
		{
			TMember TClass::* function;
			struct
			{
				void* ptr;
				void* offset;
			};
		};

		member_cast cast;

		if (sizeof(cast.function) != sizeof(cast.ptr))
		{
			return get_member_old(function);
		}

		cast.function = function;

		return (uintptr_t)cast.ptr;
	}

}
