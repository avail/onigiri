#include <stdinc.hpp>

#include <utils/HookFunction.h>
#include <utils/Hooking.h>

void SetThreadName(int dwThreadID, const char* threadName)
{
	auto SetThreadDescription = (HRESULT(WINAPI*)(HANDLE, PCWSTR))GetProcAddress(GetModuleHandle(L"kernelbase.dll"), "SetThreadDescription");

	if (SetThreadDescription)
	{
		HANDLE hThread = (dwThreadID < 0) ? GetCurrentThread() : OpenThread(THREAD_SET_LIMITED_INFORMATION, FALSE, dwThreadID);

		if (hThread != NULL)
		{
			SetThreadDescription(hThread, to_wide(threadName).c_str());

			if (dwThreadID >= 0)
			{
				CloseHandle(hThread);
			}
		}
	}
}


static HANDLE CreateThreadWrapper(_In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes, _In_ SIZE_T dwStackSize, _In_ LPTHREAD_START_ROUTINE lpStartAddress,
								  _In_opt_ __drv_aliasesMem LPVOID lpParameter, _In_ DWORD dwCreationFlags, _Out_opt_ LPDWORD lpThreadId)
{
	// find the name parameter by frobbling the parent stack
	char* parentStackPtr = reinterpret_cast<char*>(_AddressOfReturnAddress());
	char* threadName = *reinterpret_cast<char**>(parentStackPtr + 0x50 /* offset from base pointer to argument */ + 0x60 /* offset from function stack frame stack to base pointer */ + 8 /* return address offset */);

	// create metadata for passing to the thread
	struct WrapThreadMeta
	{
		char* threadName;
		LPTHREAD_START_ROUTINE origRoutine;
		void* originalData;
	};

	WrapThreadMeta* parameter = new WrapThreadMeta{ threadName, lpStartAddress, lpParameter };

	// create a thread with 'our' callback
	HANDLE hThread = CreateThread(lpThreadAttributes, dwStackSize, [](void* arguments)
	{
		// get and free metadata
		WrapThreadMeta* metaPtr = reinterpret_cast<WrapThreadMeta*>(arguments);
		WrapThreadMeta meta = *metaPtr;
		delete metaPtr;

		// set thread name, if any
		if (meta.threadName)
		{
			SetThreadName(-1, meta.threadName);

			//OPTICK_START_THREAD(meta.threadName);
		}
		else
		{
			SetThreadName(-1, "GTA5.exe UNK THREAD");
		}

		// invoke original thread start
		return meta.origRoutine(meta.originalData);
	}, parameter, dwCreationFlags, lpThreadId);

	return hThread;
}

static HookFunction hookFunction([]()
{
	// RAGE thread creation function: CreateThread call
	void* createThread = hook::pattern("48 89 44 24 28 33 C9 44 89 7C 24 20").count(1).get(0).get<void>(12);

	hook::nop(createThread, 6); // as it's an indirect call
	hook::call(createThread, CreateThreadWrapper);
});
