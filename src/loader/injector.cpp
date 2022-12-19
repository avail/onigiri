#include <stdinc.hpp>

#include <vector>
#include <tlhelp32.h>
#include <codecvt>

#include <injector.hpp>

static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> m_converter;

std::wstring to_wide(const std::string& narrow)
{
	return m_converter.from_bytes(narrow);
}

std::string to_narrow(const std::wstring& wide)
{
	return m_converter.to_bytes(wide);
}

bool injector::inject_dll(DWORD target_process, const std::wstring& library_name)
{
	auto processHandle = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, false, target_process);

	if (processHandle == NULL)
	{
		printf("couldn't open process for injecting. Run as admin?\n");
		return false;
	}

	FARPROC loadLibraryAddress = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryA");

	if (loadLibraryAddress == NULL)
	{
		printf("LoadLibraryA not found.\n");
		return false;
	}

	auto memory = VirtualAllocEx(processHandle, nullptr, to_narrow(library_name).length() + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (nullptr == memory)
	{
		printf("Couldn't allocate memory.\n");
		return false;
	}

	SIZE_T amountBytesWritten;

	auto result = WriteProcessMemory(processHandle, memory, to_narrow(library_name).c_str(), to_narrow(library_name).length() + 1, &amountBytesWritten);

	if (!result)
	{
		printf("Failed to write dll name into target process' memory space.\n");
		return false;
	}

	auto remoteThreadHandle = CreateRemoteThread(processHandle, NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(loadLibraryAddress), memory, 0, NULL);

	if (remoteThreadHandle == NULL)
	{
		printf("Couldn't create a remote thread. Can't inject dll.\n");
		return false;
	}

	//VirtualFreeEx(processHandle, memory, 0, MEM_RELEASE);
	CloseHandle(processHandle);

	return true;
}

DWORD injector::wait_for_process(const char* process_name)
{
	auto pid = get_process_id(process_name);

	while (pid == NULL)
	{
		pid = get_process_id(process_name);
		Sleep(5);
	}

	return pid;
}

DWORD injector::get_process_id(const char* process_name)
{
	auto process_name_wide = to_wide(process_name);

	HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); // all processes

	PROCESSENTRY32W entry;
	entry.dwSize = sizeof entry;

	if (!Process32FirstW(snap, &entry))
	{
		return 0;
	}

	DWORD pid = NULL;

	do
	{
		if (_wcsicmp(entry.szExeFile, &process_name_wide[0]) == 0)
		{
			pid = entry.th32ProcessID;
			break;
		}
	} while (Process32NextW(snap, &entry));


	return pid;
}
