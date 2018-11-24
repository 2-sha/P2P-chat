#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <algorithm>
#include <functional>
#include <locale>
#ifdef _WIN32
	#include <conio.h>
	#include <Windows.h>
#else
	#include <unistd.h>
	#include <termios.h>
	#include <sys/ioctl.h>
	#include <signal.h>
#endif

namespace application
{
	enum Controls
	{
		NULL_TERMINATOR = 0,
		CTR_C = 3,
#ifdef _WIN32
		BACKSPACE = 8,
		ENTER = 13,
		CTR_V = 22
#else
		BACKSPACE = 127,
		ENTER = 10
#endif
	};
	const short int MAX_INPUT_SIZE = 256;

	struct Message
	{
		std::wstring user, content;

		Message(std::wstring user, std::wstring content)
			: user(user), content(content) {};
	};

	class Chat
	{
		// Username of current user
		std::wstring username_;
		// Separator betwen message and senser name
		std::wstring separator_;
		// Separator betwen printing message and another messages
		std::wstring prompt_;
		// User input buffer
		std::wstring input_;

		// Number of characters in a user-entered message
		int inputSize_;
		// Number of lines in prompt separated by '\n'
		int promptLines_;
		// Lenght of last line of prompt
		int promptLastLine_;

		std::function<void(Message)> sendMessageCallback_;
		std::function<void()> beforeExiting_;

		// sync wcout
		std::mutex mut;

		// Removes the prompt and user-entered text
		void cleanInput();
		void sendMessage(const std::wstring &data);
		// Counts promptLines_ and promptLastLine_ for cleanInput funcion
		void calcPrompt();
		// Current cursor position + x rows rightwards and + y colums up
		void moveCursor(const int x, const int y);

		int getConsoleWidth();

#ifndef _WIN32
		int _getwch();
#endif

	public:
		Chat(const std::wstring &username, const std::wstring &separator, const std::wstring &prompt);
		Chat();

		void printMessage(Message receivedMessage);
		void printSystemMessage(std::wstring message);

		int run();

		void setSendMessageCallback(const std::function<void(Message)> &callback);
		void setBeforeExiting(const std::function<void()> &callback);
		void setPrompt(const std::wstring &prompt);
		void setSeparator(const std::wstring &separator);
		void setUsername(const std::wstring &username);
	};
}