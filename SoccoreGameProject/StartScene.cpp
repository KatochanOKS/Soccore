#include "StartScene.h"
#include "ObjectFactory.h"
#include "Transform.h"
#include "Renderer.h"
#include "Camera.h"
#include "GameScene.h"

StartScene::StartScene(EngineManager* engine) : engine(engine) {}

void StartScene::Start() {
    m_sceneObjects.clear();

    // タイトルロゴ
    int logoTex = engine->GetTextureManager()->LoadTexture(
        L"assets/Start.png",
        engine->GetDeviceManager()->GetCommandList()
    );
    GameObject* logoObj = new GameObject();
    auto* logo = logoObj->AddComponent<UIImage>();
    logo->texIndex = logoTex;
    logo->size = { 1280 , 720 };
    logo->position = { 0, 0 };
    m_sceneObjects.push_back(logoObj);
}

void StartScene::Update() {
    // 1回押し判定（ホールドで連打にならないように）
    static bool prev = false;
    bool cur = (GetAsyncKeyState(VK_RETURN) & 0x8000) != 0;
    if (cur && !prev) {
        engine->ChangeScene(std::make_unique<GameScene>(engine));
        return; // このフレームはここで終了（以降の処理は新シーンに任せる）
    }
    prev = cur;
}


void StartScene::Draw() {
    Camera* cam = engine->GetCamera();
    cam->SetPosition({ 0.0f, 3.0f, -5.0f });
    cam->SetTarget({ 0.0f, 1.0f, 0.0f });

    auto view = cam->GetViewMatrix();
    auto proj = cam->GetProjectionMatrix(
        engine->GetSwapChainManager()->GetWidth(),
        engine->GetSwapChainManager()->GetHeight()
    );

    engine->GetRenderer()->BeginFrame();
    for (size_t i = 0; i < m_sceneObjects.size(); ++i) {
        engine->GetRenderer()->DrawObject(m_sceneObjects[i], i, view, proj);
    }
    engine->GetRenderer()->EndFrame();
}

StartScene::~StartScene() {
    for (auto* obj : m_sceneObjects) delete obj;
    m_sceneObjects.clear();
}
