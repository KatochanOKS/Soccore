#include "FbxModelLoader.h"
#include "BufferManager.h"
#include <DirectXMath.h>
using namespace DirectX;
#include <algorithm>

FbxModelLoader::FbxModelLoader()
{
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
            vertices.push_back(Vertex{
                vi[0], vi[1], vi[2],
                vi[3], vi[4], vi[5],
                vi[6], 1.0f - vi[7] // �� ������ύX FBX/Blender/Maya�� �� v�͉�����オ0��1 FBX/Blender/Maya�� �� v�͉�����オ0��1
                });


        }
        // �C���f�b�N�X�̓I�t�Z�b�g��t����allIndices��
        for (auto idx : indices) {
            allIndices.push_back(idx + indexOffset);
        }
        // ���_��allVertices��
        allVertices.insert(allVertices.end(), vertices.begin(), vertices.end());
        indexOffset += static_cast<unsigned short>(vertices.size());
    }

    // �}�l�[�W���[�A�V�[���̔j��
    scene->Destroy();
    manager->Destroy();
    *vertexInfo = { allVertices, allIndices };
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

// FBX�̍s���DirectX�ɕϊ�
static XMMATRIX FbxAMatrixToXMMATRIX(const FbxAMatrix& m) {
    XMFLOAT4X4 xf;
    for (int r = 0; r < 4; r++)
        for (int c = 0; c < 4; c++)
            xf.m[r][c] = (float)m.Get(r, c);
    return XMLoadFloat4x4(&xf);
}

// �X�L�j���O���f���Ǎ�
bool FbxModelLoader::LoadSkinningModel(const std::string& filePath, SkinningVertexInfo* outInfo)
{
    // FBX������
    FbxManager* manager = FbxManager::Create();
    FbxImporter* importer = FbxImporter::Create(manager, "");
    if (!importer->Initialize(filePath.c_str(), -1, manager->GetIOSettings())) {
        manager->Destroy();
        return false;
    }
    FbxScene* scene = FbxScene::Create(manager, "");
    importer->Import(scene);
    importer->Destroy();
    FbxGeometryConverter conv(manager);
    conv.Triangulate(scene, true);

    // ������
    outInfo->vertices.clear();
    outInfo->indices.clear();
    outInfo->boneNames.clear();
    outInfo->bindPoses.clear();

    int meshCount = scene->GetSrcObjectCount<FbxMesh>();
    char buf[256];
    sprintf_s(buf, "[FBX DEBUG] meshCount = %d\n", meshCount);
    OutputDebugStringA(buf);

    bool bonesInitialized = false; // �{�[����/bindPose����x�����Z�b�g����

    unsigned int indexOffset = 0; // �������b�V���p�̒��_�I�t�Z�b�g

    for (int m = 0; m < meshCount; ++m) {
        FbxMesh* mesh = scene->GetSrcObject<FbxMesh>(m);
        if (!mesh) continue;
        int ctrlCount = mesh->GetControlPointsCount();
        int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
        int polyCount = mesh->GetPolygonCount();

        sprintf_s(buf, "[FBX DEBUG] Mesh %d: skinCount=%d, ctrlPts=%d, polys=%d\n", m, skinCount, ctrlCount, polyCount);
        OutputDebugStringA(buf);

        // --- �R���g���[���|�C���g���ƂɃ{�[�������W�߂� ---
        struct BoneWeight { int boneIdx; float weight; };
        std::vector<std::vector<BoneWeight>> ctrlptBones(ctrlCount);

        // �X�L���擾
        int clusterCount = 0;
        if (skinCount > 0) {
            FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(0, FbxDeformer::eSkin));
            clusterCount = skin->GetClusterCount();

            sprintf_s(buf, "[FBX DEBUG] Mesh %d: clusterCount=%d\n", m, clusterCount);
            OutputDebugStringA(buf);

            // �{�[����/bindPose�͍ŏ��̃X�L�j���O���b�V���ł̂݃Z�b�g�i���d�o�^�h�~�j
            if (!bonesInitialized) {
                outInfo->boneNames.resize(clusterCount);
                outInfo->bindPoses.resize(clusterCount);
            }

            for (int c = 0; c < clusterCount; ++c) {
                FbxCluster* cluster = skin->GetCluster(c);
                FbxNode* link = cluster->GetLink();
                std::string boneName = link ? link->GetName() : "Bone" + std::to_string(c);

                if (!bonesInitialized) {
                    outInfo->boneNames[c] = boneName;
                    outInfo->bindPoses[c] = FbxAMatrixToXMMATRIX(link->EvaluateGlobalTransform().Inverse());
                }

                int* indices = cluster->GetControlPointIndices();
                double* weights = cluster->GetControlPointWeights();
                int idxCount = cluster->GetControlPointIndicesCount();
                sprintf_s(buf, "[FBX DEBUG] Cluster %d: idxCount=%d (%s)\n", c, idxCount, boneName.c_str());
                OutputDebugStringA(buf);

                for (int i = 0; i < idxCount; ++i) {
                    int ctrlIdx = indices[i];
                    float w = (float)weights[i];
                    if (ctrlIdx >= 0 && ctrlIdx < ctrlCount)
                        ctrlptBones[ctrlIdx].push_back({ c, w });
                    else {
                        sprintf_s(buf, "[FBX WARNING] ctrlIdx %d out of range for ctrlCount=%d\n", ctrlIdx, ctrlCount);
                        OutputDebugStringA(buf);
                    }
                }
            }
            bonesInitialized = true;
        }
        else {
            OutputDebugStringA("[FBX DEBUG] No skin found for this mesh.\n");
        }

        // --- ���_���Ƃ�SkinningVertex�\�z ---
        FbxStringList uvSets;
        mesh->GetUVSetNames(uvSets);
        const char* uvSet = (uvSets.GetCount() > 0) ? uvSets[0] : "";

        std::vector<SkinningVertex> vertices;
        std::vector<uint16_t> indices;

        for (int pi = 0; pi < polyCount; ++pi) {
            int polySize = mesh->GetPolygonSize(pi);
            for (int vi = 0; vi < polySize; ++vi) {
                int ctrlIdx = mesh->GetPolygonVertex(pi, vi);
                FbxVector4 pos = mesh->GetControlPointAt(ctrlIdx);

                // �@��
                FbxVector4 normal;
                mesh->GetPolygonVertexNormal(pi, vi, normal);

                // UV
                FbxVector2 uv = { 0,0 }; bool unmapped = false;
                mesh->GetPolygonVertexUV(pi, vi, uvSet, uv, unmapped);

                // ���_����
                SkinningVertex v = {};
                v.x = (float)pos[0]; v.y = (float)pos[1]; v.z = (float)pos[2];
                v.nx = (float)normal[0]; v.ny = (float)normal[1]; v.nz = (float)normal[2];
                v.u = (float)uv[0]; v.v = 1.0f - (float)uv[1];

                // --- �{�[�������Z�b�g ---
                auto& boneList = ctrlptBones[ctrlIdx];
                std::sort(boneList.begin(), boneList.end(), [](auto& a, auto& b) {return a.weight > b.weight; });
                float sumW = 0.0f;
                for (int k = 0; k < 4 && k < (int)boneList.size(); ++k) {
                    v.boneIndices[k] = boneList[k].boneIdx;
                    v.boneWeights[k] = boneList[k].weight;
                    sumW += v.boneWeights[k];
                }
                if (sumW > 0.0f)
                    for (int k = 0; k < 4; ++k)
                        v.boneWeights[k] /= sumW;
                else {
                    v.boneIndices[0] = 0;
                    v.boneWeights[0] = 1.0f;
                    for (int k = 1; k < 4; ++k) {
                        v.boneIndices[k] = 0;
                        v.boneWeights[k] = 0.0f;
                    }
                }

                indices.push_back((uint16_t)vertices.size());
                vertices.push_back(v);
            }
        }

        // --- �������b�V���Ή��FindexOffset���g���Ă��炵�Ă���ǉ��I ---
        for (auto idx : indices) {
            outInfo->indices.push_back(idx + indexOffset);
        }
        outInfo->vertices.insert(outInfo->vertices.end(), vertices.begin(), vertices.end());
        indexOffset += vertices.size();
    }

    scene->Destroy();
    manager->Destroy();
    return true;
}



/// --- FBX����x�������[�h���ĕK�v�����L���b�V������֐� ---
// �i�t�@�C��������Ă΂�A���񂾂���������ɓǂݍ���ŕԂ��j
FbxModelInstance* FbxModelLoader::LoadAndCache(const std::string& fbxPath)
{
    auto* instance = new FbxModelInstance();

    // FBX�̏������i�Ǘ��N���X�E�V�[�������j
    instance->manager = FbxManager::Create();
    instance->scene = FbxScene::Create(instance->manager, "");
    FbxImporter* importer = FbxImporter::Create(instance->manager, "");

    if (!importer->Initialize(fbxPath.c_str(), -1, instance->manager->GetIOSettings())) {
        importer->Destroy();
        delete instance;
        return nullptr;
    }
    importer->Import(instance->scene);
    importer->Destroy();

    // --- �������b�V���Ή��F�ŏ��̃X�L�����b�V������{�[���E�o�C���h�|�[�Y�z��擾 ---
    int meshCount = instance->scene->GetSrcObjectCount<FbxMesh>();
    bool bonesInitialized = false;

    for (int m = 0; m < meshCount; ++m) {
        FbxMesh* mesh = instance->scene->GetSrcObject<FbxMesh>(m);
        if (!mesh) continue;
        int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);
        if (skinCount > 0 && !bonesInitialized) {
            FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(0, FbxDeformer::eSkin));
            int clusterCount = skin->GetClusterCount();

            instance->boneNames.resize(clusterCount);
            instance->bindPoses.resize(clusterCount);
            for (int c = 0; c < clusterCount; ++c) {
                FbxCluster* cluster = skin->GetCluster(c);
                FbxNode* link = cluster->GetLink();
                std::string boneName = link ? link->GetName() : "Bone" + std::to_string(c);
                instance->boneNames[c] = boneName;
                // bindPose�i�t�s��j���L���b�V��
                instance->bindPoses[c] = FbxAMatrixToXMMATRIX(link->EvaluateGlobalTransform().Inverse());
            }
            bonesInitialized = true;
        }
    }

    // --- �A�j���[�V�����̒������擾�i�擾�ł��Ȃ���΃f�t�H���g2.5�b�j ---
    instance->animationLength = 2.5; // ���̒l
    FbxAnimStack* animStack = instance->scene->GetCurrentAnimationStack();
    if (animStack) {
        FbxTakeInfo* takeInfo = instance->scene->GetTakeInfo(animStack->GetName());
        if (takeInfo) {
            FbxTimeSpan span = takeInfo->mLocalTimeSpan;
            instance->animationLength = span.GetDuration().GetSecondDouble();
        }
    }

    // �{�[����1���Ȃ����f���͎��s
    if (instance->boneNames.empty()) {
        delete instance;
        return nullptr;
    }

    return instance;
}


// --- �L���b�V���ς݃C���X�^���X�{�Đ������ŁA�{�[���ŏI�s��z����v�Z����֐� ---
void FbxModelLoader::CalcCurrentBoneMatrices(
    FbxModelInstance* instance,
    double currentTime,
    std::vector<DirectX::XMMATRIX>& outMatrices
) {
    if (!instance) return;

    // --- �A�j���[�V�����̃��[�v�Đ��ɑΉ� ---
    if (instance->animationLength > 0.0)
        currentTime = fmod(currentTime, instance->animationLength);

    // --- FBX�A�j���[�V�������C���[�擾 ---
    FbxAnimStack* animStack = instance->scene->GetCurrentAnimationStack();
    if (!animStack) return;
    FbxAnimLayer* animLayer = animStack->GetMember<FbxAnimLayer>(0);
    if (!animLayer) return;

    FbxTime time;
    time.SetSecondDouble(currentTime);

    // --- �S�{�[���ɂ��āA�O���[�o���ϊ��~�o�C���h�|�[�Y�t�s����v�Z ---
    for (int i = 0; i < (int)instance->boneNames.size(); ++i) {
        FbxNode* boneNode = instance->scene->FindNodeByName(instance->boneNames[i].c_str());
        if (!boneNode) {
            outMatrices[i] = DirectX::XMMatrixIdentity();
            continue;
        }
        FbxAMatrix globalTransform = boneNode->EvaluateGlobalTransform(time);
        DirectX::XMMATRIX current = FbxAMatrixToXMMATRIX(globalTransform);
        outMatrices[i] = DirectX::XMMatrixTranspose(instance->bindPoses[i] * current);
    }
}
