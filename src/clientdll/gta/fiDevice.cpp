#include <stdinc.hpp>
#include <set>
#include <gta/fiDevice.hpp>

void print_if_unique(const std::string& print)
{
	static std::set<std::string> arr;
	
	if (!arr.contains(print))
	{
		onigiri::services::logger::info("{}", print);
		arr.emplace(print);
	}
}

static void device_print(const char* func, const char* detail)
{
	onigiri::services::logger::debug("[{}] {}", func, detail);
}

static hook::cdecl_stub<bool* (rage::fiDevice*, const char*)> mount_wrap([]()
{
	return hook::get_pattern("44 0F B6 81 14 01 00 00", -0x1B);
});

bool rage::fiDevice::Mount(const char* mountPoint)
{
	return mount_wrap(this, mountPoint);
}

static hook::cdecl_stub<void(void*, const char*, bool, rage::fiDevice*)> set_path([]()
{
	return hook::get_pattern("41 B8 00 01 00 00 E8 ? ? ? ? 88 9E", -0x36);
});

void rage::fiDevice::SetPath(const char* path, bool allowRoot, rage::fiDevice* parent)
{
	set_path(this, path, allowRoot, parent);
}

rage::fiDeviceLocal::fiDeviceLocal()
{
	pad[0xF8] = 0;
}

rage::fiDeviceLocal::~fiDeviceLocal()
{
}

std::unordered_map<HANDLE, std::string> handleNames;

HANDLE rage::fiDeviceLocal::Open(const char* fileName, bool readOnly)
{
	device_print(__FUNCTION__, fileName);

	HANDLE handle = new std::ifstream(to_full_path(fileName), std::ios::in | std::ios::binary);
	handleNames[handle] = fileName;
	return handle;
}

uint32_t rage::fiDeviceLocal::ReadFile(HANDLE handle, void* buffer, uint32_t length)
{
	auto file = (std::ifstream*)handle;
	file->read((char*)buffer, length);

	device_print(__FUNCTION__, handleNames[handle].c_str());

	return static_cast<uint32_t>(file->gcount());
}

uint32_t rage::fiDeviceLocal::ReadBulk(HANDLE handle, uint64_t offset, char* buffer, uint32_t length)
{
	auto file = (std::ifstream*)handle;
	file->seekg(offset);

	//device_print(__FUNCTION__, handleNames[handle].c_str());

	return ReadFile(handle, buffer, length);
}

uint64_t rage::fiDeviceLocal::Seek64(HANDLE handle, int64_t distance, uint32_t method)
{
	auto file = (std::ifstream*)handle;

	if (method == SEEK_CUR)
	{
		file->seekg(distance, std::ios::cur);
	}
	else if (method == SEEK_SET)
	{
		file->seekg(distance, std::ios::beg);
	}
	else if (method == SEEK_END)
	{
		file->seekg(-1 * distance, std::ios::end);
	}

	return file->tellg();
}

uint32_t rage::fiDeviceLocal::Close(HANDLE handle)
{
	device_print(__FUNCTION__, handleNames[handle].c_str());

	auto file = (std::ifstream*)handle;
	handleNames.erase(handle);

	return true;
}

uint64_t rage::fiDeviceLocal::Size64(HANDLE handle)
{
	uint64_t fileCurrentPos = Seek64(handle, 0, SEEK_CUR);
	uint64_t fileEndPos = Seek64(handle, 0, SEEK_END);
	Seek64(handle, fileCurrentPos, SEEK_SET);

	return fileEndPos;
}

uint64_t rage::fiDeviceLocal::GetFileSize(const char* fileName)
{
	uint64_t size = 0;
	WIN32_FILE_ATTRIBUTE_DATA fileData;
	if (GetFileAttributesExA(to_full_path(fileName).c_str(), GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard, &fileData))
	{
		size = fileData.nFileSizeLow | (static_cast<size_t>(fileData.nFileSizeHigh) << 32);
	}

	//device_print(__FUNCTION__, std::format("{} {}", fileName, size).c_str());

	return size;
}

uint64_t rage::fiDeviceLocal::GetFileTime(const char* fileName)
{
	WIN32_FILE_ATTRIBUTE_DATA fileData;
	uint64_t filetime = 0;
	if (GetFileAttributesExA(to_full_path(fileName).c_str(), GET_FILEEX_INFO_LEVELS::GetFileExInfoStandard, &fileData))
	{
		filetime = *(uint64_t*)&fileData.ftLastWriteTime;
	}

	//device_print(__FUNCTION__, std::format("{} {}", fileName, filetime).c_str());

	return filetime;
}

uint32_t rage::fiDeviceLocal::GetAttributes(const char* fileName)
{
	uint32_t attributes = ::GetFileAttributesA(to_full_path(fileName).c_str());

	return attributes;
}

HANDLE rage::fiDeviceLocal::FindFileBegin(const char* path, rage::fiFindData* findData)
{
	WIN32_FIND_DATAW foundData;

	wchar_t fileName[MAX_PATH] = { 0 };

	auto pathLen = strlen(path);
	bool slashFound = false;

	if (pathLen)
	{
		if (path[pathLen - 1] == '/' || path[pathLen - 1] == '\\')
		{
			slashFound = true;
		}
	}

	wchar_t wcPath[256] = { 0 };
	MultiByteToWideChar(0xFDE9u, 0, to_full_path(path).c_str(), -1, wcPath, 256);

	wprintf_s(fileName, 256, slashFound ? L"%s*" : L"%s\\*", wcPath);
	HANDLE firstFileHandle = FindFirstFileW(fileName, &foundData);

	if (firstFileHandle == INVALID_HANDLE_VALUE)
	{
		return INVALID_HANDLE_VALUE;
	}

	ConvertFoundData(&foundData, findData);

	return firstFileHandle;
}

bool rage::fiDeviceLocal::FindFileNext(HANDLE handle, rage::fiFindData* findData)
{
	WIN32_FIND_DATAW foundData;
	bool fileFound = FindNextFileW(handle, &foundData);

	if (fileFound)
	{
		ConvertFoundData(&foundData, findData);
	}
	return fileFound;
}

int rage::fiDeviceLocal::FindFileEnd(HANDLE handle)
{
	auto file = (std::ifstream*)handle;

	return (FindClose(handle) != 0) - 1;
}

HANDLE rage::fiDeviceLocal::OpenBulk(const char* fileName, uint64_t* ptr)
{
	*ptr = 0;

	if (std::string(fileName).contains("addons/"))
	{
		print_if_unique(std::format("loading addon pack: {}", fileName));
	}

	return Open(fileName, true);
}

HANDLE rage::fiDeviceLocal::OpenBulkDrm(const char* fileName, uint64_t* ptr, void* unk)
{
	//device_print(__FUNCTION__, std::format("{} {:08x}", fileName, (uintptr_t)ptr).c_str());
	return OpenBulk(fileName, ptr);
}

uint32_t rage::fiDeviceLocal::Seek(HANDLE handle, int32_t distance, uint32_t method)
{
	return (uint32_t)Seek64(handle, (uint64_t)distance, method);
}

uint32_t rage::fiDeviceLocal::CloseBulk(HANDLE handle)
{
	//device_print(__FUNCTION__, std::format("{:08x}", (uintptr_t)handle).c_str());
	return Close(handle);
}

uint32_t rage::fiDeviceLocal::Size(HANDLE handle)
{
	//device_print(__FUNCTION__, std::format("{:08x}", (uintptr_t)handle).c_str());
	return (uint32_t)Size64(handle);
}

rage::fiDevice* rage::fiDeviceLocal::GetLowLevelDevice()
{
	return this;
}

void* rage::fiDeviceLocal::FixRelativeName(char* dest, int length, const char* source)
{
	//device_print(__FUNCTION__, std::format("{} {} {}", dest, source, length).c_str());

	memcpy(dest, source, length);
	return dest;
}

bool rage::fiDeviceLocal::SetEndOfFile(HANDLE handle)
{
	return false;
}

bool rage::fiDeviceLocal::SafeRead(HANDLE handle, void* buffer, uint32_t length)
{
	uint32_t offset = 0;

	if (length <= 0)
	{
		return true;
	}

	for (;;)
	{
		uint32_t bytesRead = ReadFile(handle, (char*)buffer + offset, length - offset);
		if (bytesRead < 0)
		{
			break;
		}

		offset += bytesRead;

		if (offset >= length)
		{
			return true;
		}
	}
	return false;
}

bool rage::fiDeviceLocal::SafeWrite(HANDLE handle, const void* buffer, uint32_t length)
{
	return false;
}

HANDLE rage::fiDeviceLocal::CreateLocal(const char* fileName)
{
	return 0;
}

HANDLE rage::fiDeviceLocal::Create(const char* fileName)
{
	return 0;
}

uint32_t rage::fiDeviceLocal::WriteBulk(HANDLE handle, uint64_t offset, const void* buffer, uint32_t length)
{
	return 0;
}

uint32_t rage::fiDeviceLocal::Write(HANDLE hande, const void* buffer, uint32_t length)
{
	return 0;
}

uint32_t rage::fiDeviceLocal::Flush(HANDLE handle)
{
	return 0;
}

bool rage::fiDeviceLocal::Delete(const char* fileName)
{
	return 0;
}

bool rage::fiDeviceLocal::Rename(const char* from, const char* to)
{
	return 0;
}

bool rage::fiDeviceLocal::MakeDirectory(const char* dir)
{
	return 0;
}

bool rage::fiDeviceLocal::UnmakeDirectory(const char* dir)
{
	return 0;
}

void rage::fiDeviceLocal::Sanitize()
{
}

bool rage::fiDeviceLocal::SetFileTime(const char* fileName, uint64_t fileTime)
{
	return 0;
}

uint64_t rage::fiDeviceLocal::GetRootDeviceId(const char*)
{
	return 1;
}

bool rage::fiDeviceLocal::SetAttributes(const char* fileName, uint32_t attributes)
{
	device_print(__FUNCTION__, std::format("{} {:08x}", fileName, attributes).c_str());
	return false;
}

uint32_t rage::fiDeviceLocal::IsMemoryMappedDevice()
{
	return 2;
}

uint32_t rage::fiDeviceLocal::GetResourceInfo(const char* fileName, rage::fiResourceInfo* flags)
{
	auto size = this->GetFileSize(fileName);
	auto file = this->Open(fileName, true);

	if (file)
	{
		struct
		{
			uint32_t magic;
			uint32_t version;
			uint32_t virtPages;
			uint32_t physPages;
		} rsc7Header = { 0 };

		this->ReadFile(file, &rsc7Header, sizeof(rsc7Header));
		this->Close(file);

		/*auto a = std::format("{}:\n"
				"\trsc7Header.magic {:08x}\n"
				"\trsc7Header.version {:08x}\n"
				"\trsc7Header.virtPages {:08x}\n"
				"\trsc7Header.physPages {:08x}", fileName, rsc7Header.magic, rsc7Header.version, rsc7Header.virtPages, rsc7Header.physPages);

		onigiri::services::logger::onigiri::services::logger::debug(a.c_str());*/

		if (rsc7Header.magic == 0x37435352) // RSC7
		{
			flags->phys = rsc7Header.physPages;
			flags->virt = rsc7Header.virtPages;

			return rsc7Header.version;
		}

		if (rsc7Header.magic == 52504637) // RPF7
		{
			return rsc7Header.version;
		}
	}

	return 0;
}

int32_t rage::fiDeviceLocal::IsValidHandle()
{
	//device_print(__FUNCTION__, "?");
	return 0;
}

uint32_t rage::fiDeviceLocal::GetBulkOffset(HANDLE handle)
{
	//device_print(__FUNCTION__, std::format("{:08x}", (uintptr_t)handle).c_str());
	return 0;
}

uint32_t rage::fiDeviceLocal::GetPhysicalSortKey(const char*)
{
	return 0x40000000;
}

bool rage::fiDeviceLocal::IsRpf()
{
	return false;
}

uint8_t rage::fiDeviceLocal::GetRpfVersion()
{
	return 0;
}

rage::fiDevice* rage::fiDeviceLocal::GetRpfDevice()
{
	return this;
}

bool rage::fiDeviceLocal::IsCloud()
{
	return false;
}

uint64_t rage::fiDeviceLocal::GetPackfileIndex()
{
	return 0;
}

const char* rage::fiDeviceLocal::GetDebugName()
{
	return "rage::fiDeviceLocal";
}

void rage::fiDeviceLocal::ConvertFoundData(LPWIN32_FIND_DATAW foundDataWin, rage::fiFindData* foundDataRage)
{
	WideCharToMultiByte(0xFDE9u, 0, foundDataWin->cFileName, -1, foundDataRage->fileName, 256, 0i64, 0i64);

	for (uint8_t i = 0; i < strlen(foundDataRage->fileName); ++i)
	{
		if (foundDataRage->fileName[i] == '/')
		{
			foundDataRage->fileName[i] = '\\';
		}
	}

	foundDataRage->lastWriteTime = foundDataWin->ftLastWriteTime;
	foundDataRage->fileAttributes = foundDataWin->dwFileAttributes;
	foundDataRage->fileSize = *(uint64_t*)(&foundDataWin->nFileSizeHigh);
}
