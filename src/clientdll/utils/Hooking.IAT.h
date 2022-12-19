#pragma once

#include <StdInc.hpp>

namespace hook
{
	template<typename TTarget, typename T>
	inline void set_call(TTarget* target, T address)
	{
		*(T*)target = get_call(address);
	}

	template<typename T>
	inline T* getRVA(HMODULE hModule, uintptr_t rva)
	{
		return (T*)((char*)hModule + rva);
	}

	template<typename TOrdinal>
	inline bool iat_matches_ordinal(HMODULE hModule, uintptr_t* nameTableEntry, TOrdinal ordinal)
	{
	}

	template<>
	inline bool iat_matches_ordinal(HMODULE hModule, uintptr_t* nameTableEntry, int ordinal)
	{
		if (IMAGE_SNAP_BY_ORDINAL(*nameTableEntry))
		{
			return IMAGE_ORDINAL(*nameTableEntry) == ordinal;
		}

		return false;
	}

	template<>
	inline bool iat_matches_ordinal(HMODULE hModule, uintptr_t* nameTableEntry, const char* ordinal)
	{
		if (!IMAGE_SNAP_BY_ORDINAL(*nameTableEntry))
		{
			auto import = getRVA<IMAGE_IMPORT_BY_NAME>(hModule, *nameTableEntry);

			return !_stricmp(import->Name, ordinal);
		}

		return false;
	}

	template<>
	inline bool iat_matches_ordinal(HMODULE hModule, uintptr_t* nameTableEntry, char* ordinal)
	{
		return iat_matches_ordinal(hModule, nameTableEntry, (const char*)ordinal);
	}

	template<typename T, typename TOrdinal>
	T iat(HMODULE hModule, const char* moduleName, T function, TOrdinal ordinal)
	{
		IMAGE_DOS_HEADER* imageHeader = (IMAGE_DOS_HEADER*)hModule;
		IMAGE_NT_HEADERS* ntHeader = getRVA<IMAGE_NT_HEADERS>(hModule, imageHeader->e_lfanew);

		IMAGE_IMPORT_DESCRIPTOR* descriptor = getRVA<IMAGE_IMPORT_DESCRIPTOR>(hModule, ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

		while (descriptor->Name)
		{
			const char* name = getRVA<char>(hModule, descriptor->Name);

			if (_stricmp(name, moduleName))
			{
				descriptor++;

				continue;
			}

			if (descriptor->OriginalFirstThunk == 0)
			{
				return nullptr;
			}

			auto nameTableEntry = getRVA<uintptr_t>(hModule, descriptor->OriginalFirstThunk);
			auto addressTableEntry = getRVA<uintptr_t>(hModule, descriptor->FirstThunk);

			while (*nameTableEntry)
			{
				if (iat_matches_ordinal(hModule, nameTableEntry, ordinal))
				{
					T origEntry = (T)*addressTableEntry;

					DWORD oldProtect;
					VirtualProtect(addressTableEntry, sizeof(T), PAGE_READWRITE, &oldProtect);

					*addressTableEntry = (uintptr_t)function;

					VirtualProtect(addressTableEntry, sizeof(T), oldProtect, &oldProtect);

					return origEntry;
				}

				nameTableEntry++;
				addressTableEntry++;
			}

			return nullptr;
		}

		return nullptr;
	}
}
