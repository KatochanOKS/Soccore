#include "EngineManager.h"
#include "GameScene.h"
#include "MeshLibrary.h"
#include"StartScene.h"
#include <memory>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

// C++でLuaのprint関数をフック
int PrintToDebug(lua_State* L) {
    int nargs = lua_gettop(L);
    std::string msg;
    for (int i = 1; i <= nargs; ++i) {
        const char* s = lua_tostring(L, i);
        if (s) msg += s;
        else msg += "<non-string>";
        if (i < nargs) msg += " ";
    }
    msg += "\n";
    OutputDebugStringA(msg.c_str());
    return 0;
}

//void TestLua() {
//    lua_State* L = luaL_newstate();
//    luaL_openlibs(L);
//
//    // Luaのprint関数をC++デバッグ出力に差し替え
//    lua_pushcfunction(L, PrintToDebug);
//    lua_setglobal(L, "print");
//
//    if (luaL_dofile(L, "assets/scripts/test.lua") != LUA_OK) {
//        const char* err = lua_tostring(L, -1);
//        OutputDebugStringA(err);
//    }
//    lua_close(L);
//}

void TestLua() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    // Luaファイルをロード
    if (luaL_dofile(L, "assets/scripts/player_config.lua") != LUA_OK) {
        const char* err = lua_tostring(L, -1);
        OutputDebugStringA(err);
        lua_close(L);
        return;
    }

    // hp取得
    lua_getglobal(L, "hp");
    int hp = (int)lua_tointeger(L, -1);
    lua_pop(L, 1);

    // speed取得
    lua_getglobal(L, "speed");
    double speed = lua_tonumber(L, -1);
    lua_pop(L, 1);

    // name取得
    lua_getglobal(L, "name");
    const char* name_utf8 = lua_tostring(L, -1);

    // UTF-8→ワイド文字列へ変換
    wchar_t nameW[128];
    MultiByteToWideChar(CP_UTF8, 0, name_utf8, -1, nameW, 128);
    lua_pop(L, 1);

    // デバッグ出力
    wchar_t buf[256];
    swprintf(buf, 256, L"Lua config: hp=%d, speed=%.2f, name=%s\n", hp, speed, nameW);
    OutputDebugStringW(buf);

    lua_close(L);
}


void EngineManager::Initialize() {
    TestLua();
    m_deviceManager.Initialize();
    auto* device = m_deviceManager.GetDevice();
    auto* cmdQueue = m_deviceManager.GetCommandQueue();
    m_swapChainManager.Initialize(m_hWnd, device, cmdQueue, 1280, 720);
    m_depthBufferManager.Initialize(device, 1280, 720);
    m_pipelineManager.Initialize(
        device,
        L"assets/VertexShader.cso", L"assets/PixelShader.cso",
        L"assets/SkinningVS.cso", L"assets/SkinningPS.cso",
        L"assets/UIVertexShader.cso", L"assets/UIPixelShader.cso"
    );

    m_textureManager.Initialize(device);

    constexpr size_t CBV_SIZE = 256;
    m_bufferManager.CreateConstantBuffer(device, CBV_SIZE * 100);

    std::vector<Vertex> quadVertices;
    std::vector<uint16_t> quadIndices;
    MeshLibrary::GetQuadMesh2D(quadVertices, quadIndices);
    m_quadBufferManager.CreateVertexBuffer(device, quadVertices);
    m_quadBufferManager.CreateIndexBuffer(device, quadIndices);
    m_quadBufferManager.CreateConstantBuffer(device, CBV_SIZE * 100);

    std::vector<Vertex> sphereVertices;
    std::vector<uint16_t> sphereIndices;
    MeshLibrary::GetSphereMesh(sphereVertices, sphereIndices, 1.0f, 32, 32);
    m_sphereBufferManager.CreateVertexBuffer(device, sphereVertices);
    m_sphereBufferManager.CreateIndexBuffer(device, sphereIndices);
    m_sphereBufferManager.CreateConstantBuffer(device, CBV_SIZE * 100);

    std::vector<Vertex> cubeVertices;
    std::vector<uint16_t> cubeIndices;
    MeshLibrary::GetCubeMesh(cubeVertices, cubeIndices);
    m_cubeBufferManager.CreateVertexBuffer(device, cubeVertices);
    m_cubeBufferManager.CreateIndexBuffer(device, cubeIndices);
    m_cubeBufferManager.CreateConstantBuffer(device, CBV_SIZE * 100);


    m_renderer.Initialize(
        &m_deviceManager,
        &m_swapChainManager,
        &m_depthBufferManager,
        &m_pipelineManager,
        &m_textureManager,
        &m_bufferManager,
        &m_modelBufferManager,
        &m_quadBufferManager,
        &m_skyBufferManager,
        &m_sphereBufferManager,
        GetModelVertexInfo()
    );

    /*m_activeScene = std::make_unique<GameScene>(this);
    m_activeScene->Start();*/

	m_activeScene = std::make_unique<StartScene>(this);
	m_activeScene->Start();

}

void EngineManager::Start() {}

void EngineManager::Update() {
    if (m_activeScene) m_activeScene->Update();
}

void EngineManager::Draw() {
    if (m_activeScene) m_activeScene->Draw();
}


void EngineManager::Shutdown() {
    m_deviceManager.Cleanup();
    m_swapChainManager.Cleanup();
}


void EngineManager::ChangeScene(std::unique_ptr<Scene> nextScene) {
    // 旧シーンは unique_ptr のムーブで自動破棄
    m_activeScene = std::move(nextScene);
    if (m_activeScene) m_activeScene->Start();
}