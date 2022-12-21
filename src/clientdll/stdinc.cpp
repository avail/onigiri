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

char to_lower(const char c)
{
	return (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;
}

uint32_t hash_string(std::string_view string)
{
	uint32_t hash = 0;

	for (char ch : string)
	{
		hash += to_lower(ch);
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

uint32_t hash_rage_string(std::string_view string)
{
	uint32_t hash = 0;

	for (char ch : string)
	{
		hash += ch;
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}
