#ifdef _MSC_VER
#define  _CRT_SECURE_NO_WARNINGS
#endif

#include <fcntl.h>
#include <cstdarg>
#include <stdexcept>

#ifdef _MSC_VER
#include <windows.h>
#ifdef ERROR
#undef ERROR
#endif

#define STDOUT_FILENO _fileno(stdout)
#define F_OK 0

#endif

#if defined(_WIN32) || defined(_MSC_VER)

#include <direct.h>
#include <io.h>

#endif

#ifdef __MINGW32__

#include <sys/stat.h>

#endif

#if defined(__linux__)

#include <unistd.h>
#include <sys/stat.h>

#endif

#include "log4cpp.hpp"
#include "file_output.h"

#include "log_pattern.h"


using namespace log4cpp;

file_output::builder &file_output::builder::set_file(const std::string &file) {
	if (this->instance == nullptr) {
		throw std::runtime_error("Call new_builder() first");
	}
	this->config.file_path = file;
	return *this;
}

std::shared_ptr<file_output> file_output::builder::build() {
	if (this->instance == nullptr) {
		throw std::runtime_error("Call new_builder() first");
	}
	if (const auto pos = this->config.file_path.find_last_of('/'); pos != std::string::npos) {
		std::string path = this->config.file_path.substr(0, pos);
		if (0 != access(path.c_str(), F_OK)) {
#if defined(_MSC_VER) || defined(_WIN32)
			(void)_mkdir(path.c_str());
#endif
#if defined(__linux__)
			mkdir(path.c_str(), 0755);
#endif
		}
	}
	int openFlags = O_RDWR | O_CREAT;
	if (this->config.append) {
		openFlags |= O_APPEND;
	}
#if defined(_MSC_VER) || defined(_WIN32)
	int mode = _S_IREAD | _S_IWRITE;
#endif

#ifdef __linux__
	openFlags |= O_CLOEXEC;
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
#endif
	this->instance->fd = open(this->config.file_path.c_str(), openFlags, mode);
	if (this->instance->fd == -1) {
		std::string what("Can not open log file, ");
		what.append(strerror(errno));
		what.append("(" + std::to_string(errno) + ")");
		throw std::runtime_error(what);
	}
	return this->instance;
}

file_output::builder file_output::builder::new_builder() {
	builder builder = file_output::builder{};
	builder.instance = std::shared_ptr<file_output>(new file_output());
	builder.config.append = true;
	return builder;
}

file_output::builder &file_output::builder::set_append(bool append) {
	this->config.append = append;
	return *this;
}

file_output::~file_output() {
	if (this->fd != -1) {
		close(this->fd);
	}
}

void file_output::log(log_level level, const char *fmt, va_list args) {
	char buffer[LOG_LINE_MAX];
	buffer[0] = '\0';
	const size_t used_len = log_pattern::format(buffer, sizeof(buffer), level, fmt, args);
	singleton_log_lock &lock = singleton_log_lock::get_instance();
	lock.lock();
	(void)write(this->fd, buffer, used_len);
	lock.unlock();
}

void file_output::log(log_level level, const char *fmt, ...) {
	char buffer[LOG_LINE_MAX];
	buffer[0] = '\0';
	const size_t used_len = log_pattern::format(buffer, sizeof(buffer), level, fmt);
	singleton_log_lock &lock = singleton_log_lock::get_instance();
	lock.lock();
	(void)write(this->fd, buffer, used_len);
	lock.unlock();
}

std::shared_ptr<file_output> file_output_config::get_instance(const file_output_config &config) {
	static std::shared_ptr<file_output> instance = nullptr;
	static log_lock instance_lock;
	if (instance == nullptr) {
		instance_lock.lock();
		if (instance == nullptr) {
			instance = file_output::builder::new_builder().set_file(config.file_path).set_append(config.append).build();
		}
		instance_lock.unlock();
	}
	return instance;
}

void log4cpp::tag_invoke(boost::json::value_from_tag, boost::json::value &json, const file_output_config &obj) {
	json = boost::json::object{
		{"filePath", obj.file_path},
		{"append", obj.append}
	};
}

file_output_config log4cpp::tag_invoke(boost::json::value_to_tag<file_output_config>, boost::json::value const &json) {
	file_output_config config;
	config.file_path = boost::json::value_to<std::string>(json.at("filePath"));
	config.append = boost::json::value_to<bool>(json.at("append"));
	return config;
}
