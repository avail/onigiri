#pragma once

#define LOG_TO_CONSOLE 0

namespace onigiri::services
{
	class logger final
	{
	public:
		enum class log_level
		{
			debug,
			info,
			warning,
			error
		};

		struct log_message
		{
			log_level level;
			std::string_view message;
		};

		static void initialize();
		static void shutdown();

		// external use
		static  __declspec(dllexport) void WriteLog(const char* channel, log_message message);

		template<typename... argz>
		static inline void info(std::format_string<argz...> fmt, argz&&... args)
		{
			write_log("onigiri", { log_level::info, &std::format(fmt, std::forward<argz>(args)...)[0] });
		}

		template<typename... argz>
		static inline void debug(std::format_string<argz...> fmt, argz&&... args)
		{
			write_log("onigiri", { log_level::debug, &std::format(fmt, std::forward<argz>(args)...)[0] });
		}

		template<typename... argz>
		static inline void error(std::format_string<argz...> fmt, argz&&... args)
		{
			write_log("onigiri", { log_level::error, &std::format(fmt, std::forward<argz>(args)...)[0] });
		}

		// logging via instance += { level, print }
		__forceinline void operator+=(log_message message)
		{
			write_log("onigiri", message);
		}

	private:
		static inline std::string time_now()
		{
			auto const time = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());
			return std::format("{:%d-%m-%Y %T}", time);
		}

		static inline void write_log(const char* channel, log_message message)
		{
			std::lock_guard _(m_log_mutex);

			std::string level_str;
			std::string level_str_console;
			switch (message.level)
			{
			case log_level::debug:
				level_str = "DEBUG";

				level_str_console = color_debug;
				break;
			case log_level::info:
				level_str = "INFO";

				level_str_console = color_info;
				break;
			case log_level::warning:
				level_str = "WARNING";

				level_str_console = color_warning;
				break;
			case log_level::error:
				level_str = "ERROR";

				level_str_console = color_error;
				break;
			}

			level_str_console += level_str;

			if (!strstr(channel, "onigiri"))
			{
				level_str_console += "[";
				level_str_console += channel;
				level_str_console += "]";
			}

			level_str_console += color_reset;
			level_str_console += '\t';

			m_log_file << time_now() << " " << level_str << " | " << message.message << std::endl;

#if LOG_TO_CONSOLE
			if (message.level != log_level::debug)
			{
				m_log_console << color_info << time_now() << " " << level_str_console << " | " << color_message << message.message << color_reset << std::endl;
			}
#endif

			OutputDebugStringA(std::format("{} {} | {}", time_now(), level_str, message.message).c_str());
		}

	private:
#if LOG_TO_CONSOLE
		static inline bool m_has_console;
		static inline HANDLE m_console_handle;
		static inline DWORD m_console_mode;
		static inline std::ofstream m_log_console;
#endif

		static inline std::mutex m_log_mutex;
		static inline std::ofstream m_log_file;

		static constexpr const char* color_reset = "\033[0m";
		static constexpr const char* color_debug = "\033[96m";
		static constexpr const char* color_info = "\033[97m";
		static constexpr const char* color_warning = "\033[93m";
		static constexpr const char* color_error = "\033[91m";
		static constexpr const char* color_message = "\033[37m";
	};
}
