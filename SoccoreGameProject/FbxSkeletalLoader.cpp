#include "FbxSkeletalLoader.h"
#include <fbxsdk.h>

// 再帰関数（インラインでもOK、staticでもOK）
void BuildBoneHierarchy(FbxNode* node, int parentIdx, std::vector<Bone>& bones) {
    if (node->GetSkeleton()) {
        Bone bone;
        bone.name = node->GetName();
        bone.parentIndex = parentIdx;
        FbxAMatrix bindPose = node->EvaluateGlobalTransform();
        bone.offsetMatrix = DirectX::XMMatrixInverse(nullptr,
            DirectX::XMMATRIX(
                (float)bindPose.Get(0, 0), (float)bindPose.Get(0, 1), (float)bindPose.Get(0, 2), (float)bindPose.Get(0, 3),
                (float)bindPose.Get(1, 0), (float)bindPose.Get(1, 1), (float)bindPose.Get(1, 2), (float)bindPose.Get(1, 3),
                (float)bindPose.Get(2, 0), (float)bindPose.Get(2, 1), (float)bindPose.Get(2, 2), (float)bindPose.Get(2, 3),
                (float)bindPose.Get(3, 0), (float)bindPose.Get(3, 1), (float)bindPose.Get(3, 2), (float)bindPose.Get(3, 3)
            )
        );
        int thisIdx = (int)bones.size();
        bones.push_back(bone);
        for (int i = 0; i < node->GetChildCount(); ++i)
            BuildBoneHierarchy(node->GetChild(i), thisIdx, bones);
    }
    else {
        for (int i = 0; i < node->GetChildCount(); ++i)
            BuildBoneHierarchy(node->GetChild(i), parentIdx, bones);
    }
}


bool FbxSkeletalLoader::LoadMesh(const std::string& filePath, SkeletalMesh& meshOut)
{
    // --- FBX初期化
    FbxManager* manager = FbxManager::Create();
    FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
    manager->SetIOSettings(ios);
    FbxImporter* importer = FbxImporter::Create(manager, "");
    if (!importer->Initialize(filePath.c_str(), -1, manager->GetIOSettings())) return false;
    FbxScene* scene = FbxScene::Create(manager, "");
    importer->Import(scene);
    importer->Destroy();

    // --- Mesh/ボーン・スキン情報の取得サンプル（詳細は後で拡張！）
    // 1. Meshノードを探す
    FbxNode* rootNode = scene->GetRootNode();
    FbxMesh* mesh = nullptr;
    for (int i = 0; i < rootNode->GetChildCount(); ++i) {
        mesh = rootNode->GetChild(i)->GetMesh();
        if (mesh) break;
    }
    if (!mesh) return false;

    // 2. 頂点座標だけまず格納（法線/UV/スキン情報は後で）
    int vertexCount = mesh->GetControlPointsCount();
    meshOut.vertices.resize(vertexCount);
    for (int i = 0; i < vertexCount; ++i) {
        FbxVector4 pos = mesh->GetControlPointAt(i);
        meshOut.vertices[i].x = (float)pos[0];
        meshOut.vertices[i].y = (float)pos[1];
        meshOut.vertices[i].z = (float)pos[2];
        // 法線・UV・スキン情報は後で
    }

    // 3. インデックス・ボーン・スキン情報も今後ここで追加
    // ...
    // mesh: FbxMesh*（上のコードの中で取得済みとする）
    int deformerCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
    for (int i = 0; i < deformerCount; i++) {
        FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(i, FbxDeformer::eSkin);
        int clusterCount = skin->GetClusterCount();
        for (int c = 0; c < clusterCount; ++c) {
            FbxCluster* cluster = skin->GetCluster(c);
            std::string boneName = cluster->GetLink()->GetName();
            int boneIdx = /*boneNameが一致するbones内インデックス取得*/ 0; // ←最初は0固定でもOK

            int* indices = cluster->GetControlPointIndices();
            double* weights = cluster->GetControlPointWeights();
            int idxCount = cluster->GetControlPointIndicesCount();
            for (int k = 0; k < idxCount; ++k) {
                int vIdx = indices[k];
                float w = (float)weights[k];

                // その頂点のboneIndices[0~3], boneWeights[0~3]の空きを見つけて格納
                for (int b = 0; b < 4; ++b) {
                    if (meshOut.vertices[vIdx].boneWeights[b] == 0.0f) {
                        meshOut.vertices[vIdx].boneIndices[b] = boneIdx;
                        meshOut.vertices[vIdx].boneWeights[b] = w;
                        break;
                    }
                }
            }
        }
    }
    meshOut.bones.clear();
    BuildBoneHierarchy(scene->GetRootNode(), -1, meshOut.bones);
    std::unordered_map<std::string, int> boneNameToIdx;
    for (int i = 0; i < meshOut.bones.size(); ++i)
        boneNameToIdx[meshOut.bones[i].name] = i;

    int deformerCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
    for (int i = 0; i < deformerCount; i++) {
        FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(i, FbxDeformer::eSkin);
        int clusterCount = skin->GetClusterCount();
        for (int c = 0; c < clusterCount; ++c) {
            FbxCluster* cluster = skin->GetCluster(c);
            std::string boneName = cluster->GetLink()->GetName();
            int boneIdx = boneNameToIdx.count(boneName) ? boneNameToIdx[boneName] : 0;

            int* indices = cluster->GetControlPointIndices();
            double* weights = cluster->GetControlPointWeights();
            int idxCount = cluster->GetControlPointIndicesCount();
            for (int k = 0; k < idxCount; ++k) {
                int vIdx = indices[k];
                float w = (float)weights[k];
                for (int b = 0; b < 4; ++b) {
                    if (meshOut.vertices[vIdx].boneWeights[b] == 0.0f) {
                        meshOut.vertices[vIdx].boneIndices[b] = boneIdx;
                        meshOut.vertices[vIdx].boneWeights[b] = w;
                        break;
                    }
                }
            }
        }
    }
    // 4. シーン/マネージャ解放
    scene->Destroy();
    manager->Destroy();
    return true;
}
