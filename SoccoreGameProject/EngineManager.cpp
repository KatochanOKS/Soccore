#include "EngineManager.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "Colors.h"
using namespace DirectX;
using namespace Colors;

void EngineManager::Initialize() {
    m_deviceManager.Initialize();
    auto* device = m_deviceManager.GetDevice();
    auto* cmdQueue = m_deviceManager.GetCommandQueue();
    m_swapChainManager.Initialize(m_hWnd, device, cmdQueue, 1280, 720);
    m_depthBufferManager.Initialize(device, 1280, 720);
    m_pipelineManager.Initialize(device, L"assets/VertexShader.cso", L"assets/PixelShader.cso");

    m_textureManager.Initialize(device);
    m_texIdx = m_textureManager.LoadTexture(L"assets/penguin1.png", m_deviceManager.GetCommandList());
    m_cubeTexIdx = m_textureManager.LoadTexture(L"assets/penguin2.png", m_deviceManager.GetCommandList());

    m_gameObjects.clear();

    // FBX���f���Ǎ�&�o�b�t�@�쐬
    if (FbxModelLoader::Load("assets/MixamoModel.fbx", &m_modelVertexInfo)) {
        m_modelBufferManager.CreateVertexBuffer(device, m_modelVertexInfo.vertices);
        m_modelBufferManager.CreateIndexBuffer(device, m_modelVertexInfo.indices);
    }

    // --- GameObject���� ---
    // FBX���f��
    {
        auto* obj = new GameObject();
        auto* tr = obj->AddComponent<Transform>();
        tr->position = XMFLOAT3(0, 0, 0);
        tr->scale = XMFLOAT3(0.05f, 0.05f, 0.05f);
        auto* mr = obj->AddComponent<MeshRenderer>();
        mr->meshType = 1;     // FBX
        mr->texIndex = -1;    // �e�N�X�`���Ȃ�
        mr->color = Red;
        m_gameObjects.push_back(obj);
    }
    // �n��
    {
        auto* obj = new GameObject();
        auto* tr = obj->AddComponent<Transform>();
        tr->position = XMFLOAT3(0, -1.0f, 0);
        tr->scale = XMFLOAT3(50.0f, 0.2f, 50.0f);
        auto* mr = obj->AddComponent<MeshRenderer>();
        mr->meshType = 0;     // �L���[�u
        mr->texIndex = m_texIdx;
        mr->color = White;
        m_gameObjects.push_back(obj);
    }
    // �L���[�u1
    {
        auto* obj = new GameObject();
        auto* tr = obj->AddComponent<Transform>();
        tr->position = XMFLOAT3(-2, 0, 0);
        tr->scale = XMFLOAT3(1, 1, 1);
        auto* mr = obj->AddComponent<MeshRenderer>();
        mr->meshType = 0;     // �L���[�u
        mr->texIndex = m_cubeTexIdx;
        mr->color = White;
        m_gameObjects.push_back(obj);
    }
    // �L���[�u2
    {
        auto* obj = new GameObject();
        auto* tr = obj->AddComponent<Transform>();
        tr->position = XMFLOAT3(2, 2, 2);
        tr->scale = XMFLOAT3(1, 1, 1);
        auto* mr = obj->AddComponent<MeshRenderer>();
        mr->meshType = 0;     // �L���[�u
        mr->texIndex = m_cubeTexIdx;
        mr->color = White;
        m_gameObjects.push_back(obj);
    }
    // ...����Cube�����l�ɒǉ�...

    // �萔�o�b�t�@
    constexpr size_t CBV_SIZE = 256;
    m_bufferManager.CreateConstantBuffer(device, CBV_SIZE * m_gameObjects.size());

    // �L���[�u�p���_�E�C���f�b�N�X�o�b�t�@
    CreateTestCube();

    // Renderer�Ɋe�}�l�[�W����n���ď�����
    m_renderer.Initialize(
        &m_deviceManager,
        &m_swapChainManager,
        &m_depthBufferManager,
        &m_pipelineManager,
        &m_textureManager,
        &m_bufferManager,
        &m_modelBufferManager,
        &m_modelVertexInfo
    );
}

void EngineManager::Start() {}
void EngineManager::Update() {}

void EngineManager::Draw() {
    static float angle = 0.0f;
    angle += 0.01f;
    XMMATRIX view = XMMatrixLookAtLH(
        XMVectorSet(0, 9, -20, 1),
        XMVectorSet(0, 0, 0, 1),
        XMVectorSet(0, 1, 0, 0)
    );
    XMMATRIX proj = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(60.0f),
        m_swapChainManager.GetWidth() / (float)m_swapChainManager.GetHeight(),
        0.1f, 100.0f
    );

    // ��]�l��Transform��
    for (auto* obj : m_gameObjects) {
        auto* tr = obj->GetComponent<Transform>();
        auto* mr = obj->GetComponent<MeshRenderer>();
        if (!tr || !mr) continue;
        if (mr->meshType == 0 && tr->scale.y < 0.3f) // �n�ʂ͉�]�Ȃ�
            tr->rotation.y = 0;
        else
            tr->rotation.y = angle;
    }

    // �萔�o�b�t�@�������݁i�e�I�u�W�F�N�g���j
    void* mapped = nullptr;
    constexpr size_t CBV_SIZE = 256;
    m_bufferManager.GetConstantBuffer()->Map(0, nullptr, &mapped);
    for (size_t i = 0; i < m_gameObjects.size(); ++i) {
        GameObject* obj = m_gameObjects[i];
        auto* tr = obj->GetComponent<Transform>();
        auto* mr = obj->GetComponent<MeshRenderer>();
        if (!tr || !mr) continue;

        ObjectCB cb;
        XMMATRIX world = tr->GetWorldMatrix();
        cb.WorldViewProj = XMMatrixTranspose(world * view * proj);
        cb.Color = mr->color;
        cb.UseTexture = (mr->texIndex >= 0 ? 1 : 0);
        memcpy((char*)mapped + CBV_SIZE * i, &cb, sizeof(cb));
    }
    m_bufferManager.GetConstantBuffer()->Unmap(0, nullptr);

    // �`�揈���iRenderer�ɈϏ��j
    m_renderer.BeginFrame();
    for (size_t i = 0; i < m_gameObjects.size(); ++i)
        m_renderer.DrawObject(m_gameObjects[i], i, view, proj);
    m_renderer.EndFrame();
}

void EngineManager::Shutdown() {
    for (auto* obj : m_gameObjects) delete obj;
    m_gameObjects.clear();
    m_deviceManager.Cleanup();
    m_swapChainManager.Cleanup();
    // �K�v�ɉ����đ�Manager��Cleanup
}

void EngineManager::CreateTestCube() {
    std::vector<Vertex> vertices = {
        // �O�� (z: -0.5, �@�� 0,0,-1)
        { -0.5f, -0.5f, -0.5f,  0, 0, -1, 0, 0 },
        { -0.5f,  0.5f, -0.5f,  0, 0, -1, 0, 1 },
        {  0.5f,  0.5f, -0.5f,  0, 0, -1, 1, 1 },
        {  0.5f, -0.5f, -0.5f,  0, 0, -1, 1, 0 },
        // �E�� (x: 0.5, �@�� 1,0,0)
        { 0.5f, -0.5f, -0.5f,   1, 0, 0,  0, 0 },
        { 0.5f,  0.5f, -0.5f,   1, 0, 0,  0, 1 },
        { 0.5f,  0.5f,  0.5f,   1, 0, 0,  1, 1 },
        { 0.5f, -0.5f,  0.5f,   1, 0, 0,  1, 0 },
        // ��� (z: 0.5, �@�� 0,0,1)
        { 0.5f, -0.5f,  0.5f,   0, 0, 1,  0, 0 },
        { 0.5f,  0.5f,  0.5f,   0, 0, 1,  0, 1 },
        { -0.5f,  0.5f,  0.5f,  0, 0, 1,  1, 1 },
        { -0.5f, -0.5f,  0.5f,  0, 0, 1,  1, 0 },
        // ���� (x: -0.5, �@�� -1,0,0)
        { -0.5f, -0.5f,  0.5f,  -1, 0, 0, 0, 0 },
        { -0.5f,  0.5f,  0.5f,  -1, 0, 0, 0, 1 },
        { -0.5f,  0.5f, -0.5f,  -1, 0, 0, 1, 1 },
        { -0.5f, -0.5f, -0.5f,  -1, 0, 0, 1, 0 },
        // ��� (y: 0.5, �@�� 0,1,0)
        { -0.5f,  0.5f, -0.5f,  0, 1, 0, 0, 1 },
        { -0.5f,  0.5f,  0.5f,  0, 1, 0, 0, 0 },
        {  0.5f,  0.5f,  0.5f,  0, 1, 0, 1, 0 },
        {  0.5f,  0.5f, -0.5f,  0, 1, 0, 1, 1 },
        // ���� (y: -0.5, �@�� 0,-1,0)
        { -0.5f, -0.5f, -0.5f,  0, -1, 0, 0, 0 },
        { -0.5f, -0.5f,  0.5f,  0, -1, 0, 0, 1 },
        {  0.5f, -0.5f,  0.5f,  0, -1, 0, 1, 1 },
        {  0.5f, -0.5f, -0.5f,  0, -1, 0, 1, 0 },
    };
    std::vector<uint16_t> indices = {
        0,1,2, 0,2,3,
        4,5,6, 4,6,7,
        8,9,10, 8,10,11,
        12,13,14, 12,14,15,
        16,17,18, 16,18,19,
        20,21,22, 20,22,23
    };
    m_bufferManager.CreateVertexBuffer(m_deviceManager.GetDevice(), vertices);
    m_bufferManager.CreateIndexBuffer(m_deviceManager.GetDevice(), indices);
}
