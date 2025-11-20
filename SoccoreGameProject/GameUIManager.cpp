#include "pch.h"
#include "GameUIManager.h"
#include "EngineManager.h"
#include "ObjectFactory.h"
#include "UIImage.h"
#include "GameObject.h"

void GameUIManager::InitUI(EngineManager* engine, std::vector<GameObject*>& sceneObjects) {

        // スカイドーム生成
        int skyTex = engine->GetTextureManager()->LoadTexture(
            L"assets/SkyDome.png",
            engine->GetDeviceManager()->GetCommandList()
        );
        auto* sky = ObjectFactory::CreateSkyDome(engine, skyTex, 600.0f, "Sky", "SkyDome");
        sceneObjects.push_back(sky);

        int logoTexRed = engine->GetTextureManager()->LoadTexture(L"assets/Red.png", engine->GetDeviceManager()->GetCommandList());
        int logoTex = engine->GetTextureManager()->LoadTexture(L"assets/Green3.png", engine->GetDeviceManager()->GetCommandList());

        // 1P用 赤残像バー（左上）
        GameObject* hp1RedObj = new GameObject();
        auto* hp1Red = hp1RedObj->AddComponent<UIImage>();
        hp1Red->m_TexIndex = logoTexRed;
        hp1Red->m_Size = { 500, 50 };
        hp1Red->m_Position = { 0, 50 };
        hp1Red->m_Color = { 1, 1, 1, 1 };
        hp1RedObj->tag = "UI";
        hp1RedObj->name = "HP1Red";
        sceneObjects.push_back(hp1RedObj);

        // 1P用 本体バー（左上）
        GameObject* hp1Obj = new GameObject();
        auto* hp1 = hp1Obj->AddComponent<UIImage>();
        hp1->m_TexIndex = logoTex;
        hp1->m_Size = { 500, 50 };
        hp1->m_Position = { 0, 50 };
        hp1->m_Color = { 1, 1, 1, 1 };
        hp1Obj->tag = "UI";
        hp1Obj->name = "HP1";
        sceneObjects.push_back(hp1Obj);

        // 2P用 赤残像バー（右上）
        GameObject* hp2RedObj = new GameObject();
        auto* hp2Red = hp2RedObj->AddComponent<UIImage>();
        hp2Red->m_TexIndex = logoTexRed;
        hp2Red->m_Size = { 500, 50 };
        hp2Red->m_Position = { 780, 50 };
        hp2Red->m_Color = { 1, 1, 1, 1 };
        hp2RedObj->tag = "UI";
        hp2RedObj->name = "HP2Red";
        sceneObjects.push_back(hp2RedObj);

        // 2P用 本体バー（右上）
        GameObject* hp2Obj = new GameObject();
        auto* hp2 = hp2Obj->AddComponent<UIImage>();
        hp2->m_TexIndex = logoTex;
        hp2->m_Size = { 500, 50 };
        hp2->m_Position = { 780, 50 };
        hp2->m_Color = { 1, 1, 1, 1 };
        hp2Obj->tag = "UI";
        hp2Obj->name = "HP2";
        sceneObjects.push_back(hp2Obj);
}