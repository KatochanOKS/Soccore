#pragma once
#include <string>
#include <DirectXMath.h>
#include "Colors.h"

class EngineManager;
class GameObject;

class ObjectFactory {
public:
    // ------------------------------------------------------------
    // すべての関数は「static」！
    // → インスタンスを作らず、どこからでも「ObjectFactory::CreateXXX()」で呼べる
    //    （道具箱・ユーティリティクラスとして使うため）
    //
    // 返り値は「GameObject*」！
    // → newで動的生成したオブジェクトのアドレスを返す
    //    （返されたポインタは呼び出し元でvector等に登録して管理する）
    // ------------------------------------------------------------

    // 例：キューブ型のオブジェクトを生成
    static GameObject* CreateCube(
        EngineManager* engine,
        const DirectX::XMFLOAT3& pos,
        const DirectX::XMFLOAT3& scale,
        int texIdx = -1,
        const DirectX::XMFLOAT4& color = Colors::White
    );

    // 例：FBXモデル（静的）の生成
    static GameObject* CreateModel(
        EngineManager* engine,
        const std::string& path,
        const DirectX::XMFLOAT3& pos,
        const DirectX::XMFLOAT3& scale,
        int texIndex = -1,
        const DirectX::XMFLOAT4& color = Colors::White
    );

    // 例：アニメ対応（スキニング）モデルの生成
    static GameObject* CreateSkinningModel(
        EngineManager* engine,
        const std::string& fbxPath,
        const DirectX::XMFLOAT3& pos,
        const DirectX::XMFLOAT3& scale,
        int texIndex = -1,
        const DirectX::XMFLOAT4& color = Colors::White
    );

    // 例：アニメ登録前のスキン対応モデル
    static GameObject* CreateSkinningBaseModel(
        EngineManager* engine,
        const std::string& fbxPath,
        const DirectX::XMFLOAT3& pos,
        const DirectX::XMFLOAT3& scale,
        int texIndex = -1,
        const DirectX::XMFLOAT4& color = Colors::White
    );

    // 例：サッカーボールの生成（サイズはscaleで調整）
    static GameObject* CreateBall(
        EngineManager* engine,
        const DirectX::XMFLOAT3& pos,
        const DirectX::XMFLOAT3& scale,
        int texIndex = -1,
        const DirectX::XMFLOAT4& color = Colors::White
    );

	// 例：スカイドーム用の球体メッシュ生成
    // ObjectFactory.h（宣言を追加）
    static GameObject* CreateSkyDome(EngineManager* engine, int texIndex, float radius = 500.0f);

};
