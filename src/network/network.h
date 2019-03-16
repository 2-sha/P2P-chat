#pragma once
#ifdef _WIN32
	#define _WIN32_WINNT 0x0501
#endif
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <thread>
#include <string>
#include <exception>
#include <vector>
#include <functional>
#include <regex>
#include <time.h>

namespace network
{
	const short int PACKAGE_SIZE = 1024;

	typedef boost::asio::ip::udp udp;
	typedef boost::asio::ip::address address;
	typedef boost::asio::ip::address_v4 address_v4;
	typedef boost::asio::socket_base socket_base;

	class Network
	{
		bool isReceiverRun = false;
		unsigned short port_;

		void receiver(const std::function<void(std::string)> callback);
	public:
		Network(const unsigned short port);
		Network();

		// Starting new thread with receiver function 
		void startReceiving(const std::function<void(std::string)> callback);
		void stopReceiving();

		// Sends a request and waits 100ms for a response that fits regexp
		std::vector<std::string> sendSurvey(const std::string &data, const std::regex &reg);

		// Sending broadcast query with data
		void sendBroadcast(const std::string &data);
		
		// Trying to connect to the port
		void setPort(const unsigned short port);
	};
}