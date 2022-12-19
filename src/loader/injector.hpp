#pragma once

class injector final
{
public:
	static DWORD wait_for_process(const char* process_name);
	static bool inject_dll(DWORD target_process, const std::wstring& library_name);

private:
	static DWORD get_process_id(const char* process_name);
};
