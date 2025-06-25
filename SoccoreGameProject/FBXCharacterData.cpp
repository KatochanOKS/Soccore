#include "FBXCharacterData.h"
#include "FBXDataContainer.h"
#include <cassert>

FBXCharacterData::~FBXCharacterData() {}

void FBXCharacterData::LoadAnimationFBX(const std::wstring& fbxPath, const std::wstring& animeLabel) {
    // FBXDataContainer を読み込む（後で実装）
    m_animeFbxMap[animeLabel] = std::make_shared<FBXDataContainer>();
    m_currentAnimeLabel = animeLabel;
}

void FBXCharacterData::SetAnime(const std::wstring& label) {
    assert(m_animeFbxMap.count(label));
    m_currentAnimeLabel = label;
    m_animeTime = 0;
}

void FBXCharacterData::UpdateAnimation() {
    auto* animeCont = m_animeFbxMap[m_currentAnimeLabel].get();
    assert(animeCont);
    int frames = animeCont->GetAnimeFrames();
    if (++m_animeTime >= frames) m_animeTime = 0;
    UpdateAnimation(m_animeTime);
}

void FBXCharacterData::UpdateAnimation(int frame) {
    auto* animeCont = m_animeFbxMap[m_currentAnimeLabel].get();
    assert(animeCont);
    animeCont->UpdateBoneMatrix(frame);
}

void FBXCharacterData::MapToGPU(BufferManager* cbBuffer, int cbIndex) {
    // 引数チェック
    assert(cbBuffer);
    assert(m_animeFbxMap.count(m_currentAnimeLabel) > 0);

    FBXDataContainer* animeCont = m_animeFbxMap[m_currentAnimeLabel].get();
    assert(animeCont);

    // スキニング行列取得
    const XMMATRIX* boneMatrices = animeCont->GetBoneMatrixPtr(); // float4x4 * 64個
    assert(boneMatrices);

    // 定数バッファマネージャからマップ用ポインタ取得
    constexpr size_t CBV_SIZE = 256;
    void* mapped = nullptr;

    cbBuffer->GetConstantBuffer()->Map(0, nullptr, &mapped);
    memcpy(static_cast<char*>(mapped) + CBV_SIZE * cbIndex, boneMatrices, sizeof(XMMATRIX) * 64);
    cbBuffer->GetConstantBuffer()->Unmap(0, nullptr);

    // GPU アドレス保存（描画時に使う）
    m_boneCBAddr = cbBuffer->GetConstantBufferGPUAddress() + CBV_SIZE * cbIndex;
}

