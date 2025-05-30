#pragma once
#include <Windows.h>
class EngineManager;

class ApplicationManager {
public:
    int Run(EngineManager* engine, HINSTANCE hInstance, int nCmdShow);

private:
    HWND m_hWnd = nullptr;
    MSG m_msg = {};

    void InitWindow(HINSTANCE hInstance, int nCmdShow);
    bool ShouldQuit();
    void PollEvents();
    int GetExitCode();
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
};
