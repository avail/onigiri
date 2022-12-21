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


#include <Hooking.h>
#include <Hooking.Invoke.h>
#include <Hooking.IAT.h>
#include <Hooking.Patterns.h>

#include <utils/static_initializer.hpp>
#include <utils/storage.hpp>
#include <utils/detour.hpp>

#include <services/logger.hpp>

inline std::atomic_bool g_running{ false };

std::wstring to_wide(const std::string& narrow);
std::string to_narrow(const std::wstring& wide);
char to_lower(const char c);
uint32_t hash_string(std::string_view string);
uint32_t hash_rage_string(std::string_view string);
