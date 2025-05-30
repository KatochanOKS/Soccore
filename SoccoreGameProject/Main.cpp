#include "ApplicationManager.h"
#include "EngineManager.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    ApplicationManager appManager;
    EngineManager engineManager;

    // EngineManagerにHWNDをセットした後、初期化
    return appManager.Run(&engineManager, hInstance, nCmdShow);
}
