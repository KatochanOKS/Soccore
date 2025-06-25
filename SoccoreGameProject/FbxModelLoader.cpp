#include "FbxModelLoader.h"
#include "BufferManager.h"
#include <map>        // �ǉ��i�{�[�������C���f�b�N�X�p�j
#include <string>     // �ǉ�
FbxModelLoader::FbxModelLoader()
{
}
// FBX�m�[�h���ċA�I�ɂ��ǂ��ă{�[�����𒊏o����֐�
void GetBonesRecursive(FbxNode* node, int parentIdx,
    std::vector<Bone>& outBones,
    std::map<std::string, int>& boneNameToIdx)
{
    if (!node) return;

    // ���̃m�[�h���u�X�P���g���i�{�[���j�v������
    if (node->GetNodeAttribute() &&
        node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton) {
        Bone bone;
        bone.name = node->GetName();    // �{�[����
        bone.parentIndex = parentIdx;   // �e�C���f�b�N�X
        bone.bindPoseMatrix = node->EvaluateGlobalTransform(); // �o�C���h�|�[�Y�s��
        int newIdx = static_cast<int>(outBones.size());
        outBones.push_back(bone);
        boneNameToIdx[bone.name] = newIdx;
        parentIdx = newIdx; // �q�̐e�Ƃ��ēn��
    }
    // �q�m�[�h���ċA�T��
    for (int i = 0; i < node->GetChildCount(); ++i) {
        GetBonesRecursive(node->GetChild(i), parentIdx, outBones, boneNameToIdx);
    }
}


bool FbxModelLoader::Load(const std::string& filePath, VertexInfo* vertexInfo)
{
    auto manager = FbxManager::Create();
    auto importer = FbxImporter::Create(manager, "");
    if (!importer->Initialize(filePath.c_str(), -1, manager->GetIOSettings()))
        return false;
    auto scene = FbxScene::Create(manager, "");
    importer->Import(scene);
    importer->Destroy();
    FbxGeometryConverter geometryConverter(manager);
    if (!geometryConverter.Triangulate(scene, true))
        return false;

    // �{�[���K�w���ċA�Ŏ��W
    std::vector<Bone> bones;
    std::map<std::string, int> boneNameToIndex;
    GetBonesRecursive(scene->GetRootNode(), -1, bones, boneNameToIndex);

    // �S�̂Ŏg���{�[�����
    std::map<std::string, int> boneNameToIndex; // �{�[�������C���f�b�N�X
    std::vector<std::string> boneNames;         // �{�[�������X�g�i�C���f�b�N�X�Ή��j


    // ======= �S���b�V���擾 =======
    int meshCount = scene->GetSrcObjectCount<FbxMesh>();
    std::vector<Vertex> allVertices;
    std::vector<unsigned short> allIndices;
    unsigned short indexOffset = 0;

    for (int m = 0; m < meshCount; ++m) {
        auto mesh = scene->GetSrcObject<FbxMesh>(m);
        if (!mesh) continue;

        // UV�Z�b�g���̎擾
        FbxStringList uvSetNameList;
        mesh->GetUVSetNames(uvSetNameList);
        if (uvSetNameList.GetCount() == 0) continue;
        const char* uvSetName = uvSetNameList.GetStringAt(0);

        // �R���g���[���|�C���g����FBX�����̒��_��
        int controlPointCount = mesh->GetControlPointsCount();
        // ���_���ƂɃX�L�����i�ő�4�̃{�[���ƃE�F�C�g�j��ۑ�����\����
        struct SkinData {
            std::vector<std::pair<int, float>> boneWeightPairs; // (�{�[���ԍ�, �E�F�C�g)
        };
        std::vector<SkinData> skinDatas(controlPointCount); // ���_����

        // ----- �X�L�����̒��o -----
        int deformerCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
        for (int deformerIdx = 0; deformerIdx < deformerCount; ++deformerIdx) {
            FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(deformerIdx, FbxDeformer::eSkin);
            if (!skin) continue;
            // ���ׂẴN���X�^�i���{�[���j�ɂ���
            for (int clusterIdx = 0; clusterIdx < skin->GetClusterCount(); ++clusterIdx) {
                FbxCluster* cluster = skin->GetCluster(clusterIdx);
                // �{�[�����̎擾
                std::string boneName = cluster->GetLink()->GetName();
                // �{�[���C���f�b�N�X���蓖�āi���o�Ȃ�ǉ��j
                int boneIdx = 0;
                if (boneNameToIndex.count(boneName) == 0) {
                    boneIdx = static_cast<int>(boneNames.size());
                    boneNames.push_back(boneName);
                    boneNameToIndex[boneName] = boneIdx;
                }
                else {
                    boneIdx = boneNameToIndex[boneName];
                }

                int* indices = cluster->GetControlPointIndices();
                double* weights = cluster->GetControlPointWeights();
                int indicesCount = cluster->GetControlPointIndicesCount();
                for (int i = 0; i < indicesCount; ++i) {
                    int vertexIdx = indices[i];
                    float weight = static_cast<float>(weights[i]);
                    if (weight <= 0.0f) continue;
                    // 4�܂œo�^
                    if (skinDatas[vertexIdx].boneWeightPairs.size() < 4) {
                        skinDatas[vertexIdx].boneWeightPairs.push_back({ boneIdx, weight });
                    }
                }
            }
        }
        // ----- �����܂łŊe���_�ɍő�4�܂ł�(�{�[���ԍ�, �E�F�C�g)���L�^����� -----

        // ���_���W
        std::vector<std::vector<float>> vertexInfoList;
        for (int i = 0; i < mesh->GetControlPointsCount(); i++) {
            auto point = mesh->GetControlPointAt(i);
            std::vector<float> vertex;
            vertex.push_back(point[0]);
            vertex.push_back(point[1]);
            vertex.push_back(point[2]);
            vertexInfoList.push_back(vertex);
        }
        // ���_���̏��
        std::vector<unsigned short> indices;
        std::vector<std::array<int, 2>> oldNewIndexPairList;
        for (int polIndex = 0; polIndex < mesh->GetPolygonCount(); polIndex++) {
            for (int polVertexIndex = 0; polVertexIndex < mesh->GetPolygonSize(polIndex); polVertexIndex++) {
                auto vertexIndex = mesh->GetPolygonVertex(polIndex, polVertexIndex);
                std::vector<float> vertexInfo = vertexInfoList[vertexIndex];
                FbxVector4 normalVec4;
                mesh->GetPolygonVertexNormal(polIndex, polVertexIndex, normalVec4);
                FbxVector2 uvVec2;
                bool isUnMapped;
                mesh->GetPolygonVertexUV(polIndex, polVertexIndex, uvSetName, uvVec2, isUnMapped);
                if (!IsExistNormalUVInfo(vertexInfo)) {
                    vertexInfoList[vertexIndex] = CreateVertexInfo(vertexInfo, normalVec4, uvVec2);
                }
                else if (!IsSetNormalUV(vertexInfo, normalVec4, uvVec2)) {
                    vertexIndex = CreateNewVertexIndex(vertexInfo, normalVec4, uvVec2, vertexInfoList, vertexIndex, oldNewIndexPairList);
                }
                indices.push_back(vertexIndex);
            }
        }
        // ���_�f�[�^���\�z���ăI�t�Z�b�g���Z
        std::vector<Vertex> vertices;
        for (int i = 0; i < vertexInfoList.size(); i++) {
            std::vector<float>& vi = vertexInfoList[i];

            Vertex v = {
                vi[0], vi[1], vi[2],    // �ʒu
                vi[3], vi[4], vi[5],    // �@��
                vi[6], 1.0f - vi[7]     // UV�iFBX��v���t�Ȃ̂�1-v�j
            };

            // ---- �X�L�j���O�i�{�[���j�����Z�b�g ----
            auto& pairs = skinDatas[i].boneWeightPairs;
            float totalWeight = 0.0f;
            for (int k = 0; k < pairs.size() && k < 4; ++k) {
                v.boneIndices[k] = pairs[k].first;   // �{�[���C���f�b�N�X
                v.boneWeights[k] = pairs[k].second;  // �E�F�C�g
                totalWeight += pairs[k].second;
            }
            // �E�F�C�g�̍��v��1.0�ɂȂ�悤���K���i�d�v�I�j
            if (totalWeight > 0.0f) {
                for (int k = 0; k < 4; ++k)
                    v.boneWeights[k] /= totalWeight;
            }
            // ---- �����܂� ----

            vertices.push_back(v);


        }
        // �C���f�b�N�X�̓I�t�Z�b�g��t����allIndices��
        for (auto idx : indices) {
            allIndices.push_back(idx + indexOffset);
        }
        // ���_��allVertices��
        allVertices.insert(allVertices.end(), vertices.begin(), vertices.end());
        indexOffset += static_cast<unsigned short>(vertices.size());
    }

    *vertexInfo = { allVertices, allIndices, bones }; // �����Ń{�[�����n��

    // �}�l�[�W���[�A�V�[���̔j��
    scene->Destroy();
    manager->Destroy();
    return true;
}

// --- �c��͂��̂܂� ---
bool FbxModelLoader::IsExistNormalUVInfo(const std::vector<float>& vertexInfo)
{
    return vertexInfo.size() == 8;
}
std::vector<float> FbxModelLoader::CreateVertexInfo(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2)
{
    std::vector<float> newVertexInfo;
    newVertexInfo.push_back(vertexInfo[0]);
    newVertexInfo.push_back(vertexInfo[1]);
    newVertexInfo.push_back(vertexInfo[2]);
    newVertexInfo.push_back(normalVec4[0]);
    newVertexInfo.push_back(normalVec4[1]);
    newVertexInfo.push_back(normalVec4[2]);
    newVertexInfo.push_back(uvVec2[0]);
    newVertexInfo.push_back(uvVec2[1]);
    return newVertexInfo;
}
int FbxModelLoader::CreateNewVertexIndex(const std::vector<float>& vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2,
    std::vector<std::vector<float>>& vertexInfoList, int oldIndex, std::vector<std::array<int, 2>>& oldNewIndexPairList)
{
    for (int i = 0; i < oldNewIndexPairList.size(); i++) {
        int newIndex = oldNewIndexPairList[i][1];
        if (oldIndex == oldNewIndexPairList[i][0]
            && IsSetNormalUV(vertexInfoList[newIndex], normalVec4, uvVec2)) {
            return newIndex;
        }
    }
    std::vector<float> newVertexInfo = CreateVertexInfo(vertexInfo, normalVec4, uvVec2);
    vertexInfoList.push_back(newVertexInfo);
    int newIndex = vertexInfoList.size() - 1;
    std::array<int, 2> oldNewIndexPair{ oldIndex , newIndex };
    oldNewIndexPairList.push_back(oldNewIndexPair);
    return newIndex;
}
bool FbxModelLoader::IsSetNormalUV(const std::vector<float> vertexInfo, const FbxVector4& normalVec4, const FbxVector2& uvVec2)
{
    return fabs(vertexInfo[3] - normalVec4[0]) < FLT_EPSILON
        && fabs(vertexInfo[4] - normalVec4[1]) < FLT_EPSILON
        && fabs(vertexInfo[5] - normalVec4[2]) < FLT_EPSILON
        && fabs(vertexInfo[6] - uvVec2[0]) < FLT_EPSILON
        && fabs(vertexInfo[7] - uvVec2[1]) < FLT_EPSILON;
}
