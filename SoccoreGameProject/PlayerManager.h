#pragma once

#include "GameObject.h"
#include "EngineManager.h"
#include <vector>
#include <string>

class PlayerManager {
public:
	/// プレイヤー生成・初期化処理

	void InitPlayers(EngineManager* engine, std::vector<GameObject*>& sceneObjects);

	/// プレイヤー状態・攻撃判定などの更新

	void UpdatePlayers();

	// プレイヤー参照

	GameObject* GetPlayer1();
	GameObject* GetPlayer2();
	GameObject* GetPlayerByName(const std::string& name);

	// 死亡判定フラグ
	bool IsP1DyingEnded() const { return m_IsP1DyingEnded; }
	bool IsP2DyingEnded() const { return m_IsP2DyingEnded; }

private:
	GameObject* m_Player1 = nullptr;
	GameObject* m_Player2 = nullptr;
	bool m_IsP1DyingEnded = false;
	bool m_IsP2DyingEnded = false;
};