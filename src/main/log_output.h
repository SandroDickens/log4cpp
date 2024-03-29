#pragma once

#include <cstddef>
#include <mutex>


#include "log4cpp.hpp"
#include "log_lock.h"

#define CONSOLE_OUTPUT_NAME  "consoleOutPut"
#define FILE_OUTPUT_NAME     "fileOutPut"
#define TCP_OUTPUT_NAME      "tcpOutPut"
#define UDP_OUTPUT_NAME      "udpOutPut"

namespace log4cpp {
	constexpr unsigned short LOG_LINE_MAX = 512;

	class log_output {
	public:
		static size_t build_prefix(log_level level, char *__restrict buf, size_t len);

		virtual void log(log_level level, const char *__restrict fmt, va_list args) = 0;

		virtual void log(log_level level, const char *__restrict fmt, ...) = 0;

		virtual ~log_output() = default;
	};

	class singleton_log_lock {
	public:
		static singleton_log_lock &get_instance() {
			static singleton_log_lock instance;
			return instance;
		}

		singleton_log_lock(const singleton_log_lock &obj) = delete;

		singleton_log_lock &operator=(const singleton_log_lock &) = delete;

		void lock() {
			_lock.lock();
		}

		void unlock() {
			_lock.unlock();
		}

	private:
		singleton_log_lock() = default;

	private:
		log_lock _lock;
	};
}
