#include "application.h"
using namespace application;

void Chat::cleanInput()
{
	int consoleWidth = getConsoleWidth(),
		lines = promptLines_ + (promptLastLine_ + input_.size() - 1) / consoleWidth;

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
	if (content.empty())
		return;
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
	calcPrompt();
}

Chat::Chat()
	: username_(L"Unknown user"), separator_(L": "), prompt_(L"---------------\nEnter you message: "), beforeExiting_([]() {})
{
	calcPrompt();
};

void Chat::printMessage(Message receivedMessage)
{
	mut.lock();
	cleanInput();
	std::wcout << receivedMessage.user << separator_ << receivedMessage.content << std::endl;
	std::wcout << prompt_ << input_;
	mut.unlock();
}

void Chat::printSystemMessage(std::wstring message)
{
	mut.lock();
	cleanInput();
	std::wcout << message << std::endl;
	std::wcout << prompt_ << input_;
	mut.unlock();
}

int Chat::run()
{
#ifdef _WIN32
	// for 'case Controls::CTR_V':
	HANDLE hData;
#endif
	int k;
	std::wstring temp;
	wchar_t ch;

	std::wcout << prompt_;

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
#ifdef _WIN32
		case Controls::CTR_V:
			if (!OpenClipboard(nullptr))
			{
				CloseClipboard();
				break;
			}
			hData = GetClipboardData(CF_UNICODETEXT);
			if (hData == nullptr)
			{
				CloseClipboard();
				break;
			}
			temp = static_cast<wchar_t*>(GlobalLock(hData));
			if (temp.empty())
			{
				CloseClipboard();
				break;
			}
			GlobalUnlock(hData);
			CloseClipboard();

			if (temp.size() + input_.size() > MAX_INPUT_SIZE)
				temp = temp.erase(MAX_INPUT_SIZE - input_.size(), temp.size() - MAX_INPUT_SIZE + input_.size());

			k = 0;
			while (temp.find(L"\r\n", k) != std::string::npos)
			{
				sendMessage(temp.substr(k, temp.find(L"\r\n", k) - k));
				k = temp.find(L"\r\n", k) + 2;
			}

			mut.lock();
			cleanInput();
			// If temp added to input_ before cleanInput(), it removes excessive lines
			input_ += temp.substr(k, temp.find(L"/n") - k);
			std::wcout << prompt_ << input_;
			mut.unlock();
			break;
#endif
		case Controls::BACKSPACE:
			if (input_.size() != 0)
			{
				input_.pop_back();
				int w = getConsoleWidth();
				if (promptLastLine_ + input_.size() % w == w)
				{
#ifdef _WIN32
					moveCursor(w - 1, 1);
					std::wcout << " \b";
					moveCursor(w - 1, 1);
#else
					moveCursor(w, 1);
					std::wcout << " ";
#endif
				}
				else
				{
					std::wcout << "\b \b";
				}
			}
			break;
		case Controls::ENTER:
			// Shipped message comes faster than the end of this case
			// cause this I reset data first and then send the message.
			if (input_.size() == 0)
				break;
			temp = input_;

			mut.lock();
			cleanInput();
			input_.clear();
			std::wcout << prompt_;
			mut.unlock();

			sendMessage(temp.c_str());
			break;
		default:
			if (input_.size() < MAX_INPUT_SIZE && ch > 31 && (ch < 128 || ch > 160))
			{
				std::wcout << ch;
				input_.push_back(ch);
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