#include <Windows.h>

#include "GameEngine.h"

int WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	Core::GameEngine::GameStart();
	Core::GameEngine::GameLoop();
	Core::GameEngine::GameEnd();
	return 0;
}
