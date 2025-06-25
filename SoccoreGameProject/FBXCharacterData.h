#pragma once
#include <fbxsdk.h>
#include <string>
#include <unordered_map>
#include <memory>
#include <DirectXMath.h>
#include "BufferManager.h"

class FBXDataContainer;

class FBXCharacterData {
public:
    FBXCharacterData() = default;
    ~FBXCharacterData();

    void LoadAnimationFBX(const std::wstring& fbxPath, const std::wstring& animeLabel);
    void SetAnime(const std::wstring& label);
    void UpdateAnimation(); // �A�j���[�V������i�߂�
    void UpdateAnimation(int frame); // �w��t���[���ɃX�L�������킹��
    void MapToGPU(BufferManager* cbBuffer, int cbIndex); // CBV��������
    void SetBufferManager(BufferManager* buffer) { m_cbBuffer = buffer; }
    BufferManager* GetBufferManager() const { return m_cbBuffer; }
private:
    std::unordered_map<std::wstring, std::shared_ptr<FBXDataContainer>> m_animeFbxMap;
    std::wstring m_currentAnimeLabel;
    int m_animeTime = 0;

    BufferManager* m_cbBuffer = nullptr; // �X�L�j���O�s��p�o�b�t�@�}�l�[�W��
};
