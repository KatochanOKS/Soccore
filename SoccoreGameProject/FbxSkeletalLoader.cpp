#include "FbxSkeletalLoader.h"
#include <fbxsdk.h>

// �ċA�֐��i�C�����C���ł�OK�Astatic�ł�OK�j
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
    // --- FBX������
    FbxManager* manager = FbxManager::Create();
    FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
    manager->SetIOSettings(ios);
    FbxImporter* importer = FbxImporter::Create(manager, "");
    if (!importer->Initialize(filePath.c_str(), -1, manager->GetIOSettings())) return false;
    FbxScene* scene = FbxScene::Create(manager, "");
    importer->Import(scene);
    importer->Destroy();

    // --- Mesh/�{�[���E�X�L�����̎擾�T���v���i�ڍׂ͌�Ŋg���I�j
    // 1. Mesh�m�[�h��T��
    FbxNode* rootNode = scene->GetRootNode();
    FbxMesh* mesh = nullptr;
    for (int i = 0; i < rootNode->GetChildCount(); ++i) {
        mesh = rootNode->GetChild(i)->GetMesh();
        if (mesh) break;
    }
    if (!mesh) return false;

    // 2. ���_���W�����܂��i�[�i�@��/UV/�X�L�����͌�Łj
    int vertexCount = mesh->GetControlPointsCount();
    meshOut.vertices.resize(vertexCount);
    for (int i = 0; i < vertexCount; ++i) {
        FbxVector4 pos = mesh->GetControlPointAt(i);
        meshOut.vertices[i].x = (float)pos[0];
        meshOut.vertices[i].y = (float)pos[1];
        meshOut.vertices[i].z = (float)pos[2];
        // �@���EUV�E�X�L�����͌��
    }

    // 3. �C���f�b�N�X�E�{�[���E�X�L���������ケ���Œǉ�
    // ...
    // mesh: FbxMesh*�i��̃R�[�h�̒��Ŏ擾�ς݂Ƃ���j
    int deformerCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
    for (int i = 0; i < deformerCount; i++) {
        FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(i, FbxDeformer::eSkin);
        int clusterCount = skin->GetClusterCount();
        for (int c = 0; c < clusterCount; ++c) {
            FbxCluster* cluster = skin->GetCluster(c);
            std::string boneName = cluster->GetLink()->GetName();
            int boneIdx = /*boneName����v����bones���C���f�b�N�X�擾*/ 0; // ���ŏ���0�Œ�ł�OK

            int* indices = cluster->GetControlPointIndices();
            double* weights = cluster->GetControlPointWeights();
            int idxCount = cluster->GetControlPointIndicesCount();
            for (int k = 0; k < idxCount; ++k) {
                int vIdx = indices[k];
                float w = (float)weights[k];

                // ���̒��_��boneIndices[0~3], boneWeights[0~3]�̋󂫂������Ċi�[
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
    // 4. �V�[��/�}�l�[�W�����
    scene->Destroy();
    manager->Destroy();
    return true;
}
