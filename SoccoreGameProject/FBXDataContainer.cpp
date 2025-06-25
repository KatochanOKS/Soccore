#include "FBXDataContainer.h"
#include <cassert>
#include <DirectXMath.h>

using namespace DirectX;

FBXDataContainer::FBXDataContainer() {}
FBXDataContainer::~FBXDataContainer() {
    if (m_scene) m_scene->Destroy();
    if (m_manager) m_manager->Destroy();
}

bool FBXDataContainer::Load(const std::wstring& fbxPath) {
    m_manager = FbxManager::Create();
    FbxIOSettings* ios = FbxIOSettings::Create(m_manager, IOSROOT);
    m_manager->SetIOSettings(ios);

    FbxImporter* importer = FbxImporter::Create(m_manager, "");
    if (!importer->Initialize(std::string(fbxPath.begin(), fbxPath.end()).c_str(), -1, ios)) return false;

    m_scene = FbxScene::Create(m_manager, "scene");
    importer->Import(m_scene);
    importer->Destroy();

    // アニメーションの取得
    FbxAnimStack* animStack = m_scene->GetCurrentAnimationStack();
    m_animLayer = animStack->GetMember<FbxAnimLayer>();
    FbxTimeSpan timeSpan = animStack->GetLocalTimeSpan();
    m_totalFrames = static_cast<int>(timeSpan.GetDuration().GetFrameCount(FbxTime::eFrames60));

    // ボーン情報の抽出
    ExtractBoneHierarchy(m_scene->GetRootNode());

    // 初期値でボーン行列作成
    m_finalMatrices.resize(m_bones.size(), XMMatrixIdentity());
    return true;
}

int FBXDataContainer::GetAnimeFrames() const {
    return m_totalFrames;
}

void FBXDataContainer::ExtractBoneHierarchy(FbxNode* node) {
    if (!node) return;

    if (node->GetNodeAttribute() && node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
        Bone bone;
        bone.name = node->GetName();
        bone.node = node;
        bone.offsetMatrix = XMMatrixIdentity(); // 本来はBindPoseから逆行列を取得
        m_bones.push_back(bone);
    }

    for (int i = 0; i < node->GetChildCount(); ++i)
        ExtractBoneHierarchy(node->GetChild(i));
}

void FBXDataContainer::UpdateBoneMatrix(int frame) {
    for (size_t i = 0; i < m_bones.size(); ++i) {
        Bone& bone = m_bones[i];

        // 各ボーンのグローバル変形行列を取得
        FbxTime time;
        time.SetFrame((int)frame, FbxTime::eFrames60);

        FbxAMatrix fbxMatrix = bone.node->EvaluateGlobalTransform(time);

        // DX用に変換
        XMMATRIX m = XMMatrixIdentity();
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 4; c++) {
                m.r[r].m128_f32[c] = static_cast<float>(fbxMatrix[r][c]);
            }
        }

        // スキニング用最終行列（ここでは単純にTransformのみ）
        m_finalMatrices[i] = XMMatrixTranspose(m); // HLSLはrow-majorなのでtranspose
    }
}
