#include "network.h"
using namespace network;

Network::Network(const unsigned short port)
{
	setPort(port);
}

Network::Network()
	: port_(0)
{
}

void Network::receiver(const std::function<void(std::string)> callback)
{
	boost::asio::io_service service;
	udp::socket sock(service, udp::endpoint(udp::v4(), port_));
	char buf[PACKAGE_SIZE];
	while (true)
	{
		try
		{
			udp::endpoint sender_ep;
			memset(buf, 0, PACKAGE_SIZE);

			sock.receive_from(boost::asio::buffer(buf, PACKAGE_SIZE), sender_ep);

			if (!isReceiverRun)
				return;

			callback(buf);
		}
		catch (...)
		{
			continue;
		}
	}
}

void Network::startReceiving(const std::function<void(std::string)> callback)
{
	if (isReceiverRun)
		return;

	std::thread receiverThread(&Network::receiver, this, callback);
	receiverThread.detach();
	isReceiverRun = true;
}

void Network::stopReceiving()
{
	isReceiverRun = false;
}

std::vector<std::string> Network::sendSurvey(const std::string &data, const std::regex &reg)
{
	boost::asio::io_service service;
	char buf[PACKAGE_SIZE];
	memset(buf, 0, PACKAGE_SIZE);
	std::vector<std::string> responses;

	sendBroadcast(data);
	// Accept incoming connections that match the regular expression for 100ms
	udp::socket receivingSock(service, udp::endpoint(udp::v4(), port_));
	boost::asio::deadline_timer timer(service, boost::posix_time::milliseconds(200));
	timer.async_wait(
		[&service](const boost::system::error_code &ex) { service.stop(); }
	);
	std::function<void(const boost::system::error_code&, std::size_t)> onRead =
		[&](const boost::system::error_code &err, std::size_t read_bytes)
	{
		if (std::regex_match(buf, reg))
			responses.push_back(buf);

		memset(buf, 0, PACKAGE_SIZE);
		receivingSock.async_receive(boost::asio::buffer(buf, PACKAGE_SIZE), onRead);
	};
	receivingSock.async_receive(boost::asio::buffer(buf, PACKAGE_SIZE), onRead);

	service.run();
	return responses;
}

void Network::sendBroadcast(const std::string &data)
{
	boost::asio::io_service service;
	udp::endpoint ep(address_v4::broadcast(), port_);
	udp::socket senderSock(service, udp::endpoint(udp::v4(), 0));
	senderSock.set_option(socket_base::broadcast(true));

	senderSock.send_to(boost::asio::buffer(data, PACKAGE_SIZE), ep);

	senderSock.close();
}

void Network::setPort(const unsigned short port)
{
	boost::asio::io_service service;
	port_ = port;
	std::string testQuery = std::to_string(time(0));
	std::string res;
	char buf[PACKAGE_SIZE];
	memset(buf, 0, PACKAGE_SIZE);

	// Timer waits receiver start
	boost::asio::deadline_timer queryTimer(service, boost::posix_time::milliseconds(100));
	queryTimer.async_wait(
		[&](const boost::system::error_code &ex) { sendBroadcast(testQuery); }
	);
	
	// Receive test query
	udp::socket sock(service, udp::endpoint(udp::v4(), port_));
	boost::asio::deadline_timer stopTimer(service, boost::posix_time::milliseconds(200));
	stopTimer.async_wait(
		[&service](const boost::system::error_code &ex) { service.stop(); }
	);
	std::function<void(const boost::system::error_code&, std::size_t)> onRead =
		[&](const boost::system::error_code &err, std::size_t read_bytes)
	{
		res = buf;
		memset(buf, 0, PACKAGE_SIZE);
		sock.async_receive(boost::asio::buffer(buf, PACKAGE_SIZE), onRead);
	};
	sock.async_receive(boost::asio::buffer(buf, PACKAGE_SIZE), onRead);
	service.run();

	if (testQuery != res)
		throw std::runtime_error("Access denied");
}
