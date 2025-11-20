#include "pch.h"
#include "ApplicationManager.h"
#include "EngineManager.h"

#include "imgui.h"
#include "imgui_impl_win32.h"   // ← backends/ は付けない

// ★ これを追加（前方宣言）
extern LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


void ApplicationManager::InitWindow(HINSTANCE hInstance, int nCmdShow) {
    WNDCLASSEX windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    windowClass.lpfnWndProc = ApplicationManager::WndProc;
    windowClass.hInstance = hInstance;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    //                ↑ ここがウィンドウの初期背景色
    windowClass.lpszClassName = L"GAME_WINDOW";
    RegisterClassEx(&windowClass);

    m_hWnd = CreateWindowEx(
        0, TEXT("GAME_WINDOW"), L"DX12 Game",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
}

int ApplicationManager::Run(EngineManager* engine, HINSTANCE hInstance, int nCmdShow) {
    InitWindow(hInstance, nCmdShow);

    // HWNDをEngineManagerへセット
    engine->SetHWND(m_hWnd);
    engine->Initialize();

    // メインループ
    engine->Start();
    while (!ShouldQuit()) {
        PollEvents();
         engine->Update();
        engine->Draw();
    }
    engine->Shutdown();
    return GetExitCode();
}

bool ApplicationManager::ShouldQuit() {
    return m_msg.message == WM_QUIT;
}

void ApplicationManager::PollEvents() {
    if (PeekMessage(&m_msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&m_msg);
        DispatchMessage(&m_msg);
    }
}

int ApplicationManager::GetExitCode() {
    return static_cast<int>(m_msg.wParam);
}

LRESULT CALLBACK ApplicationManager::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    // ★ ImGui にもメッセージを渡す
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp))
        return true;

    switch (msg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
}
