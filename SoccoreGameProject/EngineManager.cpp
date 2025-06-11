#include "EngineManager.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "Colors.h"
#include "ObjectFactory.h" // ���ǉ�
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

    ID3D12GraphicsCommandList* cmdList = m_deviceManager.GetCommandList();
    int groundTex = m_textureManager.LoadTexture(L"assets/penguin2.png", cmdList);
    int playerTex = m_textureManager.LoadTexture(L"assets/penguin1.png", cmdList);
    int cubeTex = m_textureManager.LoadTexture(L"assets/penguin2.png", cmdList);
    int enemyTex = m_textureManager.LoadTexture(L"assets/penguin2.png", cmdList);

    m_gameObjects.clear();

    // ---- Unity���z�u ----
    ObjectFactory::CreateCube(this, { 0, -1.0f, 0 }, { 50, 0.2f, 50 }, groundTex, White);      // �n��
    ObjectFactory::CreateCube(this, { 0,  0.0f, 0 }, { 1, 1, 1 }, playerTex, White);           // �v���C���[
    ObjectFactory::CreateCube(this, { -2,  0.0f, 0 }, { 1, 1, 1 }, cubeTex, White);             // Cube1
    ObjectFactory::CreateCube(this, { 2,  2.0f, 2 }, { 1, 1, 1 }, cubeTex, White);             // Cube2
    ObjectFactory::CreateModel(this, "assets/MixamoModel.fbx", { 0, 0, 0 }, { 0.05f, 0.05f, 0.05f }, Red); // FBX���f��

    // �萔�o�b�t�@
    constexpr size_t CBV_SIZE = 256;
    m_bufferManager.CreateConstantBuffer(device, CBV_SIZE * m_gameObjects.size());


    m_renderer.Initialize(
        &m_deviceManager,
        &m_swapChainManager,
        &m_depthBufferManager,
        &m_pipelineManager,
        &m_textureManager,
        &m_bufferManager,         // Cube�p�o�b�t�@
        &m_modelBufferManager,    // ���f���p�o�b�t�@
        &m_bufferManager,         // ���萔�o�b�t�@�i���ʂŎg���Ă�Ȃ炱���OK�j
        GetModelVertexInfo()
    );

}


void EngineManager::Start() {}
void EngineManager::Update() {}

void EngineManager::Draw() {
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

    // �萔�o�b�t�@�������݁iTransform�Ɋ�Â�World�s��j
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
