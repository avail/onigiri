#pragma once

#include <windows.h>
#include <cstdint>
#include <thread>
#include <functional>
#include <format>
#include <fstream>
#include <mutex>
#include <memory>
#include <stack>
#include <atomic>
#include <map>

#include <chrono>
using namespace std::literals;

#include <MinHook.h>

#define DISABLE_COPYING(T)				\
	T(const T &) = delete;				\
	void operator=(const T &) = delete


#include <utils/static_initializer.hpp>
#include <utils/storage.hpp>
#include <utils/Hooking.Patterns.h>
#include <utils/detour.hpp>

#include <services/logger.hpp>

inline std::atomic_bool g_running{ false };

std::wstring to_wide(const std::string& narrow);
std::string to_narrow(const std::wstring& wide);


inline constexpr char ToLower(const char c)
{
	return (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c;
}

inline constexpr uint32_t HashString(std::string_view string)
{
	uint32_t hash = 0;

	for (char ch : string)
	{
		hash += ToLower(ch);
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}

	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);

	return hash;
}

inline constexpr uint32_t HashRageString(std::string_view string)
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
