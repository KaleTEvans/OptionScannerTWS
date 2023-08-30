#pragma once
#pragma warning( disable : 4275 )

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#define DEBUG_LOGS

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

#include "Enums.h"

#include <memory>

#define OPTIONSCANNER_DEFAULT_LOGGER_NAME "optiondatalog"

class Logger {
public:
	static void Initialize() {
		auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		consoleSink->set_pattern("[%Y-%m-%d %H:%M:%S] %^[ line: %# ]%$ %v");
		consoleSink->set_level(spdlog::level::info);

		auto fileLogger = std::make_shared<spdlog::sinks::basic_file_sink_mt>("../logs/logs.txt");
		fileLogger->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] [%@] %v");

		std::vector<spdlog::sink_ptr> sinks{ consoleSink, fileLogger };
		option_data_logs = std::make_shared<spdlog::logger>(OPTIONSCANNER_DEFAULT_LOGGER_NAME, sinks.begin(), sinks.end());
		option_data_logs->set_level(spdlog::level::trace);
		option_data_logs->flush_on(spdlog::level::trace);
		spdlog::register_logger(option_data_logs);
		//spdlog::set_default_logger(option_data_logs);

		/*auto debugFileLogs = std::make_shared<spdlog::sinks::basic_file_sink_mt>("../logs/debug.txt");
		debugFileLogs->set_pattern("[%Y-%m-%d %H:%M:%S] %^[ %s ] [ %! ] [ line: %# ]%$ %v");*/

		auto debugConsoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		debugConsoleSink->set_pattern("[%Y-%m-%d %H:%M:%S] %^[ %s ] [ %! ] [ line: %# ]%$ %v");
		debugConsoleSink->set_level(spdlog::level::debug);

		//std::vector<spdlog::sink_ptr> debugSinks{ debugConsoleSink, debugFileLogs };
		//debug_logs = std::make_shared<spdlog::logger>("debuglogs", sinks.begin(), sinks.end());
		debug_logs = std::make_shared<spdlog::logger>("debuglogs", debugConsoleSink);
		debug_logs->set_level(spdlog::level::debug);
		debug_logs->flush_on(spdlog::level::debug);
		spdlog::register_logger(debug_logs);
	}

	static void Shutdown() {
		spdlog::shutdown();
	}

	static std::shared_ptr<spdlog::logger>& getCoreLogger() { return option_data_logs; }
	static std::shared_ptr<spdlog::logger>& getDebugLogger() { return debug_logs; }

private:
	static std::shared_ptr<spdlog::logger> option_data_logs;
	static std::shared_ptr<spdlog::logger> debug_logs;
};

#define OPTIONSCANNER_TRACE(...) SPDLOG_LOGGER_TRACE(Logger::getCoreLogger(), __VA_ARGS__)

#ifdef DEBUG_LOGS
#define OPTIONSCANNER_DEBUG(...) SPDLOG_LOGGER_DEBUG(Logger::getDebugLogger(), __VA_ARGS__)
#else
#define OPTIONSCANNER_DEBUG(...) (void)0
#endif

#define OPTIONSCANNER_INFO(...) SPDLOG_LOGGER_INFO(Logger::getCoreLogger(), __VA_ARGS__)
#define OPTIONSCANNER_WARN(...) SPDLOG_LOGGER_WARN(Logger::getCoreLogger(), __VA_ARGS__)
#define OPTIONSCANNER_ERROR(...) SPDLOG_LOGGER_ERROR(Logger::getCoreLogger(), __VA_ARGS__)
#define OPTIONSCANNER_FATAL(...) SPDLOG_LOGGER_CRITICAL(Logger::getCoreLogger(), __VA_ARGS__)