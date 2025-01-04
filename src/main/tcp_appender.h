#pragma once

#include <unordered_set>
#include <memory>
#include <thread>
#include <boost/json.hpp>

#include "log_net.h"
#include "log_appender.h"

namespace log4cpp {
	class tcp_appender;

	class tcp_appender_config {
	public:
		net::net_addr local_addr{};
		unsigned short port{0};
		static log_lock instance_lock;
		static std::shared_ptr<tcp_appender> instance;
	public:
		/**
		 * @brief Get an instance of tcp_appender with the given configuration
		 * @param config TCP appender configuration
		 * @return TCP appender instance
		 */
		static std::shared_ptr<tcp_appender> get_instance(const tcp_appender_config &config);

		friend void tag_invoke(boost::json::value_from_tag, boost::json::value &json, tcp_appender_config const &obj);

		friend tcp_appender_config
		tag_invoke(boost::json::value_to_tag<tcp_appender_config>, boost::json::value const &json);
	};

	void tag_invoke(boost::json::value_from_tag, boost::json::value &json, tcp_appender_config const &obj);

	tcp_appender_config tag_invoke(boost::json::value_to_tag<tcp_appender_config>, boost::json::value const &json);


	class tcp_appender final : public log_appender {
	public:
		class builder {
		public:
			/**
			 * @brief Set the local address for the TCP appender
			 * @param addr Local address
			 * @return Reference to the builder
			 */
			builder &set_local_addr(const net::net_addr &addr);

			/**
			 * @brief Set the port for the TCP appender
			 * @param port Port number
			 * @return Reference to the builder
			 */
			builder &set_port(unsigned short port);

			/**
			 * @brief Build the TCP appender instance
			 * @return TCP appender instance
			 */
			std::shared_ptr<tcp_appender> build();

			/**
			 * @brief Create a new TCP appender builder
			 * @return TCP appender builder instance
			 */
			static builder new_builder();

		private:
			builder() = default;

		private:
			tcp_appender_config config;
			std::shared_ptr<tcp_appender> instance{nullptr};
		};

		tcp_appender(const tcp_appender &other) = delete;

		tcp_appender(tcp_appender &&other) = delete;

		tcp_appender &operator=(const tcp_appender &other) = delete;

		tcp_appender &operator=(tcp_appender &&other) = delete;

		/**
		 * @brief Write log message with the given log level
		 * @param level Log level
		 * @param fmt Format string
		 * @param args Arguments
		 */
		void log(log_level level, const char *__restrict fmt, va_list args) override;

		/**
		 * @brief Write log message with the given log level
		 * @param level Log level
		 * @param fmt Format string
		 * @param ... Arguments
		 */
		void log(log_level level, const char *__restrict fmt, ...) override;

		~tcp_appender() override;

	private:
		tcp_appender();

	public:
		/* TCP server accept thread running flag */
		static std::atomic_bool running;

	private:
		/* TCP server listen socket */
		net::socket_fd fd;
		/* Socket fds of connected clients */
		std::unordered_set<net::socket_fd> clients;
		/* Thread for accepting client connections */
		std::thread accept_thread;
		log_lock lock;
	};
}