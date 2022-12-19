#include <stdinc.hpp>

#include <services/logger.hpp>

namespace onigiri::services
{
	void logger::initialize()
	{
		AllocConsole();
		m_console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

		SetConsoleTitleA("grcWindow");
		SetConsoleOutputCP(CP_UTF8);

		DWORD console_mode;
		GetConsoleMode(m_console_handle, &console_mode);
		m_console_mode = console_mode;

		// terminal like behaviour enable full color support
		console_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
		// prevent clicking in terminal from suspending our main thread
		console_mode &= ~(ENABLE_QUICK_EDIT_MODE);

		SetConsoleMode(m_console_handle, console_mode);

		m_log_file.open(utils::storage::get_relative_storage_path("onigiri.log"), std::ios::binary);
		m_log_console.open("CONOUT$", std::ios_base::out | std::ios_base::app);
	}

	void logger::shutdown()
	{
		m_log_file.close();
		m_log_console.close();

		SetConsoleMode(m_console_handle, m_console_mode);
		FreeConsole();
	}
}
