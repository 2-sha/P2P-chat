#include "application.h"
using namespace application;

void Chat::cleanInput()
{
	int consoleWidth = getConsoleWidth(),
		lines = promptLines_ + (promptLastLine_ + inputSize_ - 1) / consoleWidth;

	for (int j = 0; j < lines; j++)
	{
		std::wcout << "\r";
		for (int i = 0; i < consoleWidth; i++)
			std::wcout << " ";
		std::wcout << "\r";
		if (j != promptLines_ - 1)
		{
#ifdef _WIN32
			moveCursor(0, 2);
#else
			moveCursor(0, 1);
#endif
		}
	}
#ifdef _WIN32
	moveCursor(0, 1);
#endif
}

int Chat::getConsoleWidth()
{
#ifdef _WIN32
	HANDLE hWndConsole;
	hWndConsole = GetStdHandle(-12);
	CONSOLE_SCREEN_BUFFER_INFO sbi;
	GetConsoleScreenBufferInfo(hWndConsole, &sbi);
	return sbi.srWindow.Right - sbi.srWindow.Left + 1;
#else
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	return w.winsize::ws_col;
#endif
}

#ifndef _WIN32
int Chat::_getwch()
{
	long long int ch;
	struct termios oldt, newt;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	ch = getwchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;
}
#endif

void Chat::sendMessage(const std::wstring &content)
{
	sendMessageCallback_(Message(username_, content));
}

void Chat::calcPrompt()
{
	promptLines_ = 1;
	int lastN, i;
	for (i = 0; i < prompt_.size(); i++)
	{
		if (prompt_[i] == '\n')
		{
			promptLines_++;
			lastN = i;
		}
	}
	promptLastLine_ = i - lastN;
}

void Chat::moveCursor(const int x, const int y)
{
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO sbi;
	COORD position;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &sbi);
	position.X = sbi.dwCursorPosition.X + x;
	position.Y = sbi.dwCursorPosition.Y - y;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), position);
#else
	if (x > 0)
		std::wcout << "\033[" << x << "C";
	else if (x < 0)
		std::wcout << "\033[" << abs(x) << "D";

	if (y > 0)
		std::wcout << "\033[" << y << "A";
	else if (y < 0)
		std::wcout << "\033[" << abs(y) << "B";
#endif
}

Chat::Chat(const std::wstring &username, const std::wstring &separator, const std::wstring &prompt) 
	: username_(username), separator_(separator), prompt_(prompt), beforeExiting_([]() {})
{
	_input[MAX_INPUT_SIZE - 1] = 0;
	calcPrompt();
}

Chat::Chat()
	: username_(L"Unknown user"), separator_(L": "), prompt_(L"---------------\nEnter you message: "), beforeExiting_([]() {})
{
	_input[MAX_INPUT_SIZE - 1] = 0;
	calcPrompt();
};

void Chat::printMessage(Message receivedMessage)
{
	mut.lock();
	cleanInput();
	std::wcout << receivedMessage.user << separator_ << receivedMessage.content << std::endl;
	std::wcout << prompt_ << _input;
	mut.unlock();
}

void Chat::printSystemMessage(std::wstring message)
{
	mut.lock();
	cleanInput();
	std::wcout << message << std::endl;
	std::wcout << prompt_ << _input;
	mut.unlock();
}

int Chat::run()
{
	std::wstring temp;
	wchar_t ch;

	std::wcout << prompt_;
	inputSize_ = 0;

	while (true)
	{
		ch = _getwch();

		switch (ch)
		{
		case Controls::NULL_TERMINATOR:
			continue;
			break;
		case Controls::CTR_C:
			beforeExiting_();
			return 0;
		case Controls::BACKSPACE:
			if (inputSize_ != 0)
			{
				_input[inputSize_--] = 0;
				int w = getConsoleWidth();
				if (promptLastLine_ + inputSize_ % w == w)
				{
#ifdef _WIN32
					moveCursor(w - 1, 1);
					std::wcout << " \b";
					moveCursor(w - 1, 1);
#else
					std::wcout << "\b \b";
#endif
				}
				else
				{
					std::wcout << "\b \b";
				}
			}
			break;
		case Controls::ENTER:
			// The message may come faster than the end of this case
			// so I reset the data first and then send the message.
			if (inputSize_ == 0)
				break;
			temp = _input;

			mut.lock();
			cleanInput();
			wmemset(_input, 0, MAX_INPUT_SIZE);
			inputSize_ = 0;
			std::wcout << prompt_;
			mut.unlock();

			sendMessage(temp);
			break;
		default:
			if (inputSize_ < MAX_INPUT_SIZE && 
				ch > 31 && (ch < 128 || ch > 160))
			{
				std::wcout << ch;
				_input[inputSize_++] = ch;
			}
			break;
		}
	}
}

void Chat::setPrompt(const std::wstring &prompt)
{
	prompt_ = prompt;
	promptLines_ = 1;

	calcPrompt();
}

void Chat::setSeparator(const std::wstring &separator)
{
	separator_ = separator;
}

void Chat::setUsername(const std::wstring &username)
{
	username_ = username;
}

void Chat::setSendMessageCallback(const std::function<void(Message)> &callback)
{
	sendMessageCallback_ = callback;
}

void Chat::setBeforeExiting(const std::function<void()> &callback)
{
	beforeExiting_ = callback;
}