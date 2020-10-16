// main.cpp : �� ���Ͽ��� 'main' �Լ��� ���Ե˴ϴ�. �ű⼭ ���α׷� ������ ���۵ǰ� ����˴ϴ�.
//
#include <Windows.h>
#include <iostream>
#include <conio.h>
#include <ctime>
#include <cstdlib> 
#include <deque>
#include <string>
#include <vector>
#include "Utils.h"

using namespace std;

// forward declaration

class Screen {
	int width;
	int height;
	char* buffer;

	Screen(int w = 10, int h = 10)
		: width(w), height(h), buffer{ new char[getSize()] }
	{	
		Borland::initialize();
		buffer[getSize() - 1] = '\0';
	}
	static Screen* instance;

public:

	static Screen* getInstance() {
		if (instance == nullptr) {
			instance = new Screen{ 80, 20 };
		}
		return instance;
	}

	~Screen() { delete[] buffer; }

	int getWidth() const { return width; }
	int getScreenWidth() const { return width + 1; }
	int getHeight() const { return height; }
	int getSize() const { return getScreenWidth()*height; }

	void clear() { memset(buffer, ' ', getSize()); buffer[getSize() - 1] = '\0';  }

	void draw(int x, int y, char shape) { buffer[y* getScreenWidth() + x] = shape; }
	void draw(int x, int y, const char* shape) {
		strncpy(&buffer[y* getScreenWidth() + x], shape, strlen(shape));
	}

	void render()
	{
		for (int y = 0; y < height - 1; y++) {
			buffer[y * getScreenWidth() + width] = '\n';
		}
		buffer[getSize()-1] = '\0';

		Borland::gotoxy(0, 0);
		cout << buffer;
	}
};

Screen* Screen::instance = nullptr;

// �̱���
// Ŭ������ ������ �ν��Ͻ��� �ϳ��� �����ϵ��� ����� ��.
// ������ �Լ��� �� �ѹ� �θ��� ��
// �̱��� ����
// 0. ������ �Լ��� private�ϰ� ����
// 1. �ܺο� ����� getInstance��� static �Լ��� ����ϵ��� �����Ѵ�.(������ �ּ� ��ȯ)
// 2. ���� ������ ��ü �ν��Ͻ��� ������ instance ������ private�ϰ� ����

// ����ϴ� ����
// ���� ������� �ʴ´ٸ� �ƿ� �ν��Ͻ��� �������� �ʴ´�.
// ��Ÿ�ӿ� �ʱ�ȭ�ȴ�.
// �̱����� ����� �� �ִ�.


// singleton
class InputManager {

	INPUT_RECORD irInBuf[128];

	HANDLE hStdin;
	DWORD fdwSaveOldMode;

	deque<INPUT_RECORD> events;

	InputManager() : hStdin(GetStdHandle(STD_INPUT_HANDLE)), irInBuf{ {0} }	{
		if (hStdin == INVALID_HANDLE_VALUE) return;
		FlushConsoleInputBuffer(hStdin);
		if (!GetConsoleMode(hStdin, &fdwSaveOldMode)) return;		
		if (!SetConsoleMode(hStdin, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT)) return;
		events.clear();
	}

	VOID ErrorExit(const char* lpszMessage)
	{
		Borland::gotoxy(0, 21);
		printf("%80d\r", ' ');
		printf("%s\n", lpszMessage);

		// Restore input mode on exit.

		SetConsoleMode(hStdin, fdwSaveOldMode);

		ExitProcess(0);
	}

	VOID KeyEventProc(KEY_EVENT_RECORD ker)
	{
		if (ker.bKeyDown)
			printf("key pressed\n");
		else 
			printf("key released\n");
	}

	VOID MouseEventProc(MOUSE_EVENT_RECORD mer)
	{
#ifndef MOUSE_HWHEELED
#define MOUSE_HWHEELED 0x0008
#endif
		Borland::gotoxy(0, 22);
		printf("%80d\r", ' ');

		printf("Mouse event: %d %d      ", mer.dwMousePosition.X, mer.dwMousePosition.Y);

		switch (mer.dwEventFlags)
		{
		case 0:

			if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
			{
				printf("left button press \n");
			}
			else if (mer.dwButtonState == RIGHTMOST_BUTTON_PRESSED)
			{
				printf("right button press \n");
			}
			else
			{
				printf("button press\n");
			}
			break;
		case DOUBLE_CLICK:
			printf("double click\n");
			break;
		case MOUSE_HWHEELED:
			printf("horizontal mouse wheel\n");
			break;
		case MOUSE_MOVED:
			printf("mouse moved\n");
			break;
		case MOUSE_WHEELED:
			printf("vertical mouse wheel\n");
			break;
		default:
			printf("unknown\n");
			break;
		}

	}


	static InputManager* instance;
public:
	static InputManager* getInstance() {
		if (instance == nullptr) {
			instance = new InputManager;
		}
		return instance;
	}

	bool GetKeyDown(WORD ch) {
		if (events.empty() == true) return false;
		const INPUT_RECORD& in = events.front();
		if (in.EventType != KEY_EVENT) return false;
		if (in.Event.KeyEvent.bKeyDown == TRUE) {
			return in.Event.KeyEvent.wVirtualKeyCode == ch;
		}
		return false;
	}

	void readInputs() {
		DWORD cNumRead;
		DWORD nEvents;

		if (!GetNumberOfConsoleInputEvents(hStdin, &nEvents)) return;
		if (nEvents == 0) return;

		nEvents = min(nEvents, 128);
		ReadConsoleInput(
			hStdin,      // input buffer handle 
			irInBuf,     // buffer to read into 
			nEvents,         // size of read buffer 
			&cNumRead); // number of records read

		for (int i = 0; i < (int)cNumRead; i++)
		{
			events.push_back(irInBuf[i]);
		}
	}

	void consumeEvent()
	{
		if (events.empty() == true) return;
		events.pop_front();
	}
};

class GameObject {

	string shape;
	Position pos;

protected:
	Screen& screen;
	InputManager& inputManager;

public:
	GameObject(int x, int y, const string& shape)
		: shape(shape), screen(*Screen::getInstance()),
		inputManager(*InputManager::getInstance()),
		pos(x, y)
	{
		start();
	}
	virtual ~GameObject() {}

	void setPos(int x, int y) { pos.x = x; pos.y = y; }
	void setPos(const Position& pos) { this->pos.x = pos.x; this->pos.y = pos.y; }
	Position getPos() const { return pos; }

	virtual void update() {}
	virtual void start() {}
	virtual void draw() { screen.draw(pos.x, pos.y, shape.c_str()); }
};

class Block : public GameObject {

public:
	Block(const string& shape = "(^_^)", int x = 5, int y = 5)
		: GameObject(x, y, shape) {}

	void update() override
	{
		Position pos = getPos();

		if (inputManager.GetKeyDown(VK_LEFT) == true) {
			setPos(pos - Position::right);
		}
		if (inputManager.GetKeyDown(VK_RIGHT) == true) {
			setPos(pos + Position::right);
		}
		if (inputManager.GetKeyDown(VK_UP) == true) {
			setPos(pos - Position::up);
		}
		if (inputManager.GetKeyDown(VK_DOWN) == true) {
			setPos(pos + Position::up);
		}
		if (inputManager.GetKeyDown(VK_MULTIPLY) == true) {
			setPos(pos * 2);
		}
		if (inputManager.GetKeyDown(VK_DIVIDE) == true) {
			setPos(pos / 2);
		}
	}
};

InputManager* InputManager::instance = nullptr;

int main()
{
	Screen& screen = *Screen::getInstance();
	InputManager& inputManager = *InputManager::getInstance();
	
	bool requestExit = false;
	int x = 0, y = 0;
	vector<GameObject*> gameObjects;
	gameObjects.push_back(new Block{ "(^_^)", 5, 5 });
	gameObjects.push_back(new Block{ "(-_-)", 10, 10 });

	

	while (requestExit == false)
	{
		screen.clear();

		inputManager.readInputs();

		for (auto object : gameObjects)
		{
			object->update();
		}
		for (auto object : gameObjects)
		{
			object->draw();
		}

		screen.render();

		inputManager.consumeEvent();
		
		Sleep(100);
	}
	printf("\n���������� ����Ǿ����ϴ�.\n");
	return 0;
}

