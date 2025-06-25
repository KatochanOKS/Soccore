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
    void UpdateAnimation(); // アニメーションを進める
    void UpdateAnimation(int frame); // 指定フレームにスキンを合わせる
    void MapToGPU(BufferManager* cbBuffer, int cbIndex); // CBV書き込み
    void SetBufferManager(BufferManager* buffer) { m_cbBuffer = buffer; }
    BufferManager* GetBufferManager() const { return m_cbBuffer; }
private:
    std::unordered_map<std::wstring, std::shared_ptr<FBXDataContainer>> m_animeFbxMap;
    std::wstring m_currentAnimeLabel;
    int m_animeTime = 0;

    BufferManager* m_cbBuffer = nullptr; // スキニング行列用バッファマネージャ
};
