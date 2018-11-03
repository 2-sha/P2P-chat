#include "network.h"
using namespace network;

Network::Network(const unsigned short port)
	: localIp(getLocalIp())
{
	setPort(port);
}

Network::Network()
	: port_(0), localIp(getLocalIp())
{
}

void Network::receiver(const std::function<void(std::string)> callback)
{
	udp::socket sock(service, udp::endpoint(udp::v4(), port_));
	char buf[PACKAGE_SIZE];
	while (true)
	{
		try
		{
			udp::endpoint sender_ep;
			memset(buf, 0, PACKAGE_SIZE);

			sock.receive_from(boost::asio::buffer(buf, PACKAGE_SIZE), sender_ep);

			// Do not accept your own requests
			if (sender_ep.address() == localIp)
				continue;
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

address Network::getLocalIp()
{
	udp::resolver resolver(service);
	// Any host that is always running
	udp::resolver::query query(udp::v4(), "google.com", "");
	udp::resolver::iterator endpoints = resolver.resolve(query);
	udp::endpoint ep = *endpoints;
	udp::socket socket(service);
	socket.connect(ep);
	return socket.local_endpoint().address();
}

void Network::startReceiving(const std::function<void(std::string)> callback)
{
	if (isReceiverRun)
		throw std::runtime_error("Receiver already run!");

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
	char buf[PACKAGE_SIZE];
	memset(buf, 0, PACKAGE_SIZE);
	std::vector<std::string> responses;

	sendBroadcast(data);

	// Accept incoming connections that match the regular expression for 100ms
	udp::socket receivingSock(service, udp::endpoint(udp::v4(), port_));
	boost::asio::deadline_timer timer(service, boost::posix_time::milliseconds(100));
	timer.async_wait(
		[&receivingSock](const boost::system::error_code &ex) { receivingSock.close(); }
	);
	std::function<void(const boost::system::error_code&, std::size_t)> onRead =
		[&](const boost::system::error_code &err, std::size_t read_bytes)
	{
		if (!receivingSock.is_open())
			return;
		
		if (regex_match(buf, reg))
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
	udp::endpoint ep(address_v4::broadcast(), port_);
	udp::socket senderSock(service, udp::endpoint(udp::v4(), 0));
	senderSock.set_option(socket_base::broadcast(true));

	senderSock.send_to(boost::asio::buffer(data, PACKAGE_SIZE), ep);

	senderSock.close();
}

void Network::setPort(const unsigned short port)
{
	port_ = port;
	// Trying to setup port
	udp::socket sock(service, udp::endpoint(udp::v4(), port));
}
