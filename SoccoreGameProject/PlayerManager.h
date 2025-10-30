#pragma once

#include "GameObject.h"
#include "EngineManager.h"
#include <vector>
#include <string>

class PlayerManager {
public:
	/// �v���C���[�����E����������

	void InitPlayers(EngineManager* engine, std::vector<GameObject*>& sceneObjects);

	/// �v���C���[��ԁE�U������Ȃǂ̍X�V

	void UpdatePlayers();

	// �v���C���[�Q��

	GameObject* GetPlayer1();
	GameObject* GetPlayer2();
	GameObject* GetPlayerByName(const std::string& name);

	// ���S����t���O
	bool IsP1DyingEnded() const { return m_IsP1DyingEnded; }
	bool IsP2DyingEnded() const { return m_IsP2DyingEnded; }

private:
	GameObject* m_Player1 = nullptr;
	GameObject* m_Player2 = nullptr;
	bool m_IsP1DyingEnded = false;
	bool m_IsP2DyingEnded = false;
};