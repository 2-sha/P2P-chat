// Show all warning messages
//#define DEBUG_MODE
#define BOOST_ASIO_ENABLE_HANDLER_TRACKING

#include <iostream>
#include <string>
#include <vector>
#include <codecvt>
#include <locale>
#include <regex>
#include "json.hpp"
#include "network/network.h"
#include "application/application.h"
#ifdef _WIN32
	#include <io.h>
	#include <fcntl.h>
	#include <Windows.h>
#else
	#include <iconv.h>
#endif
typedef nlohmann::json json;
typedef application::Message Message;

// Because I can't capture variables in the lambda function in SetConsoleCtrlHandler
// I have to make the variables global
network::Network net;
application::Chat app;

std::wstring username;
unsigned short int port = 8001;

int main(int argc, char* argv[])
{
	// Now messages are displayed in the console without waiting for input '\n'
	setvbuf(stdout, 0, _IONBF, 0);
	setvbuf(stderr, 0, _IONBF, 0);

	// By default, boost displays error messages in the system language
#ifdef _WIN32
	SetThreadUILanguage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
#endif

	// Set encoding to UTF-16 (UTF-8 on Linux)
#ifdef _WIN32
	_setmode(_fileno(stdout), _O_U16TEXT);
	_setmode(_fileno(stdin), _O_U16TEXT);
	_setmode(_fileno(stderr), _O_U16TEXT);
#else
	setlocale(LC_ALL, "");
#endif

	for (int i = 0; i < argc; i++)
	{
		if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "-port") == 0)
		{
			try
			{
				if (argc >= i + 1)
					port = std::stoi(argv[++i]);
			}
			catch (...)
			{
				continue;
			}
		}
	}

	// Trying connect to the socket
	try
	{
		net.setPort(port);
	}
	catch (std::exception &ex)
	{
		std::wcout << "Unable connect to " << port << " port" << std::endl;
		std::wcout << ex.what() << std::endl;
#ifdef _WIN32
		std::wcin.ignore(255, '\n');
		std::wcin.get();
#endif
		return 0;
	}
	std::wcout << "Connection to port " << port << " was successful" << std::endl << std::endl;

	// Prining list of online users
	std::vector<std::string> userList = net.sendSurvey(json({
				{ "type", "user_list_req" }
		}).dump(), std::regex(".*(\"type\":\"user_list_res\").*"));
	if (userList.empty())
		std::wcout << "You are alone at this port :(" << std::endl << std::endl;
	else
	{
		std::wcout << "List of online user:" << std::endl;
		for (auto it : userList)
		{
			try
			{
				std::wcout << json::parse(it).at("data").at("user").get<std::wstring>() << std::endl;
			}
			catch (std::exception &ex)
			{
				std::wcout << "Parsing username error!" << std::endl;
				continue;
			}
		}
		std::wcout << std::endl;
	}

	// Username setting
	std::wcout << "Welcome to our perfect chat!\nEnter your username: ";
	bool isUsernameCorrect;
	do
	{
		isUsernameCorrect = true;
		try
		{
			std::getline(std::wcin, username);

			if (username.size() > 15)
			{
				std::wcout << "The username is too long!\nTry to enter another username: ";
				isUsernameCorrect = false;
				continue;
			}
			if (username.empty())
			{
				std::wcout << "Username can't be empty!\nTry to enter another username: ";
				isUsernameCorrect = false;
				continue;
			}


			std::vector<std::string> userList = net.sendSurvey(json({
				{ "type", "user_list_req" }
			}).dump(), std::regex(".*(\"type\":\"user_list_res\").*"));
	
			for (auto it : userList)
			{
				try
				{
					if (json::parse(it).at("data").at("user").get<std::wstring>() == username)
					{
						std::wcout << "Username already exists!\nTry to enter another username: ";
						isUsernameCorrect = false;
						break;
					}
				}
				catch (std::exception &ex)
				{
					continue;
				}
			}
		}
		catch (std::exception &ex)
		{
			std::wcout << "Username validation error!\nDescriptiom: " << ex.what() << std::endl;
#ifdef _WIN32
			std::wcin.ignore(255, '\n');
			std::wcin.get();
#endif
			return 0;
		}
	} while (isUsernameCorrect == false);
	app.setUsername(username);

	std::wcout << std::endl;

	// If everything all rigth, talking another users, that we are here
	try
	{
		net.sendBroadcast(json({
		{ "type", "add_user" },
			{ "data", {
				{ "user", username }
			}}
		}).dump());
	}
	catch (std::exception &ex)
	{
		std::wcout << "Unable add to the chat!\nDescriptiom: " << ex.what() << std::endl;
#ifdef _WIN32
		std::wcin.ignore(255, '\n');
		std::wcin.get();
#endif
		return 0;
	}

	// If chat wants to send message, it calls this function
	app.setSendMessageCallback([](Message message)
	{
		try
		{
			net.sendBroadcast(json({
				{ "type", "message" },
				{ "data", {
					{ "user", message.user },
					{ "content", message.content }
				}}
			}).dump());
		}
		catch (std::exception &ex)
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
			app.printSystemMessage(
				L"\nUnable to send message\nDescription: " +
				converter.from_bytes(ex.what()) + L'\n'
			);
		}
	});

	// If receptor gets the message, it calls this function
	net.startReceiving([](std::string package)
	{
		if (package.empty())
			return;

		try
		{
			json data = json::parse(package);

			// Get type of query
			std::string type = data.at("type").get<std::string>();

			// It's a new message, we print it
			if (type == "message")
			{
				app.printMessage(Message(
					data.at("data").at("user").get<std::wstring>(),
					data.at("data").at("content").get<std::wstring>()
				));
			}
			// Somebody want's list of online users
			else if (type == "user_list_req")
			{
				net.sendBroadcast(json({
					{ "type", "user_list_res" },
					{ "data", {
						{ "user", username }
					}}
				}).dump());
			}
			// New user joined, we print a system message about it
			else if (type == "add_user")
			{
				app.printSystemMessage(data.at("data").at("user").get<std::wstring>() + L" connected");
			}
			// Someone from users left chat, we print a system message about it
			else if (type == "remove_user")
			{
				app.printSystemMessage(data.at("data").at("user").get<std::wstring>() + L" disconnected");
			}
		}
		catch (std::exception &ex)
		{
#ifdef DEBUG_MODE
			std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
			app.printSystemMessage(
				L"\nError with incoming message!\nDescription: " +
				converter.from_bytes(ex.what()) + L'\n'
			);
#else
			return;
#endif
		}
	});

	// Ctrl+c processing on Windows and Linux
#ifdef _WIN32
	SetConsoleCtrlHandler([](DWORD eventCode) -> BOOL WINAPI
	{
		if (eventCode == CTRL_CLOSE_EVENT || eventCode == CTRL_SHUTDOWN_EVENT)
		{
			net.stopReceiving();
			net.sendBroadcast(json({
			{ "type", "remove_user" },
				{ "data", {
					{ "user", username }
				}}
			}).dump());
			return 0;
		}
		return 1;
	}, TRUE);
#else
	signal(SIGINT, [](int eventCode)
	{
		net.stopReceiving();
		net.sendBroadcast(json({
		{ "type", "remove_user" },
			{ "data", {
				{ "user", username }
			}}
		}).dump());

		// Transfer of the terminal to canonical mode
		struct termios tattr;
		tcgetattr(STDIN_FILENO, &tattr);
		tattr.c_lflag = (ICANON | ECHO);
		tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);

		exit(0);
	});
#endif 

	// Starts application cycle
	app.setBeforeExiting([]()
	{
		net.stopReceiving();
		net.sendBroadcast(json({
		{ "type", "remove_user" },
			{ "data", {
				{ "user", username }
			}}
		}).dump());
	});
	return app.run();
}