#include <stdinc.hpp>

#include <codecvt>

static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> m_converter;

std::wstring to_wide(const std::string& narrow)
{
	return m_converter.from_bytes(narrow);
}

std::string to_narrow(const std::wstring& wide)
{
	return m_converter.to_bytes(wide);
}
