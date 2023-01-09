#include <stdinc.hpp>

#include <codecvt>

std::wstring to_wide(const std::string& narrow)
{
	if (narrow.empty())
	{
		return L"";
	}

	auto narrow_size = static_cast<std::int32_t>(narrow.size());
	auto wide_size = MultiByteToWideChar(CP_UTF8, 0, &narrow[0], narrow_size, NULL, 0);

	std::wstring wide(wide_size, 0);
	MultiByteToWideChar(CP_UTF8, 0, &narrow[0], narrow_size, &wide[0], wide_size);
	return wide;
}

std::string to_narrow(const std::wstring& wide)
{
	if (wide.empty())
	{
		return "";
	}

	auto wide_size = static_cast<std::int32_t>(wide.size());
	auto narrow_size = WideCharToMultiByte(CP_UTF8, 0, &wide[0], wide_size, NULL, 0, NULL, NULL);

	std::string narrow(narrow_size, 0);
	WideCharToMultiByte(CP_UTF8, 0, &wide[0], wide_size, &narrow[0], narrow_size, NULL, NULL);
	return narrow;
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
