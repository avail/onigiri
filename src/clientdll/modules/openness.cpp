#include <stdinc.hpp>

#if 0
#include <gta/fiPackfile.hpp>

#include <utils/Hooking.h>
#include <utils/Hooking.IAT.h>
#include <utils/HookFunction.h>

struct DirectoryEntry
{
	uint32_t nameOffset;
	uint32_t entryType;
	uint32_t entryIndex;
	uint32_t entriesCnt;
};

struct BinaryEntry
{
	uint16_t nameOffset;
	uint8_t fileSize[3];
	uint8_t fileOffset[3];
	uint32_t realSize;
	uint32_t isEncrypted;

	uint32_t GetFileSize()
	{
		return fileSize[0] + (fileSize[1] << 8) + (fileSize[2] << 16);
	}

	uint32_t GetFileOffset()
	{
		return fileOffset[0] + (fileOffset[1] << 8) + (fileOffset[2] << 16);
	}

	bool IsCompressed()
	{
		return GetFileSize() != 0;
	}
};

struct ResourceEntry
{
	uint16_t nameOffset;
	uint8_t fileSize[3];
	uint8_t fileOffset[3];
	uint32_t systemFlags;
	uint32_t graphicsFlags;

	uint32_t GetFileSize()
	{
		return fileSize[0] + (fileSize[1] << 8) + (fileSize[2] << 16);
	}

	uint32_t GetFileOffset()
	{
		return (fileOffset[0] + (fileOffset[1] << 8) + (fileOffset[2] << 16)) & 0x7FFFFF;
	}
};

struct Entry
{
	union
	{
		DirectoryEntry dir;
		BinaryEntry bin;
		ResourceEntry res;
	};

	bool IsDirectory()
	{
		return dir.entryType == 0x7FFFFF00;
	}

	bool IsBinary()
	{
		return !IsDirectory() && (dir.entryType & 0x80000000) == 0L;
	}

	bool IsResource()
	{
		return !IsDirectory() && !IsBinary();
	}
};

uint32_t currentEncryption;
bool FindEncryptionHook(uint32_t encryption)
{
	currentEncryption = encryption;
	return (encryption & 0xFF00000) == 0xFE00000;
}

void(*g_origDecryptHeader)(uint32_t, char*, int);
void DecryptHeaderHook(uint32_t salt, char* entryTable, int size)
{
	if (currentEncryption == 0x4E45504F)
	{
		onigiri::services::logger::debug("found NEPO packfile, not decrypting");
		return;
	}

	g_origDecryptHeader(salt, entryTable, size);
}

void(*g_origDecryptHeader2)(uint32_t, uint32_t, char*, int);
void DecryptHeader2Hook(uint32_t encryption, uint32_t salt, char* header, int nameTableLen)
{
	if (encryption == 0x4E45504F)
	{
		onigiri::services::logger::debug("found NEPO packfile, not decrypting");
		return;
	}

	g_origDecryptHeader2(encryption, salt, header, nameTableLen);
}

bool(*g_origParseHeader)(rage::fiPackfile*, const char*, bool, void*);
bool ParseHeaderHook(rage::fiPackfile* a1, const char* name, bool readHeader, void* customHeader)
{
	bool ret = g_origParseHeader(a1, name, readHeader, customHeader);

	if (ret)
	{
		for (int i = 0; i < a1->filesCount; ++i)
		{
			Entry* v21 = (Entry*)(a1->entryTable + 16 * i);
			if (v21->IsBinary() && v21->bin.nameOffset > 0 && v21->bin.isEncrypted)
			{
				if (currentEncryption == 0x4E45504F)
				{
					onigiri::services::logger::debug("ParseHeaderHook oiv package");
					v21->bin.isEncrypted = 0xFEFFFFF;
				}
			}
		}

		if (currentEncryption == 0x4E45504F)
		{
			onigiri::services::logger::debug("ParseHeaderHook oiv package");
			a1->currentFileOffset = 0xFEFFFFF;
		}
	}

	return ret;
}

static HookFunction _([]
{
	onigiri::services::logger::info("we are NEPO");

	hook::call(hook::get_pattern("E8 ? ? ? ? 48 8B 53 20 44 8B C7 41 8B CE E8"), FindEncryptionHook);

	{
		auto location = hook::get_address<void*>(hook::get_pattern("E8 ? ? ? ? 41 8B D4 44 39 63 28 76 3F 41 B9", 1));
		hook::set_call(&g_origDecryptHeader, location);
		hook::call(location, DecryptHeaderHook);
	}

	{
		auto location = hook::get_address<void*>(hook::get_pattern("E8 ? ? ? ? 8B 55 F8 48 8B 43 10 48 03 D0 48 8B CB 48 89 53 18 66 44 89 22 33 D2 E8", 1));
		hook::set_call(&g_origDecryptHeader2, location);
		hook::call(location, DecryptHeader2Hook);
	}

	{
		auto location = hook::get_address<void*>(hook::get_pattern("44 88 BB ? ? ? ? 89 43 58 E8 ? ? ? ? 4C 8D 9C 24 ? ? ? ? 49 8B 5B 38 49 8B 73 40 49 8B 7B 48 49 8B E3 41 5F 41 5E 41 5D 41 5C 5D C3", 11));
		hook::set_call(&g_origParseHeader, location);
		hook::call(location, ParseHeaderHook);
	}
});
#endif
