#include "EngineManager.h"
#include "BufferManager.h" // Vertex���g������
#include <DirectXMath.h>
#include"GameObject.h"
using namespace DirectX;
using namespace Colors;
void EngineManager::Initialize() {
    // �萔�o�b�t�@�p�̍\���́i16�o�C�g���E�̂��߃p�f�B���O�s�v�Afloat4x4+float4�j
   
    m_deviceManager.Initialize();
    auto* device = m_deviceManager.GetDevice();
    auto* cmdQueue = m_deviceManager.GetCommandQueue();
    m_swapChainManager.Initialize(m_hWnd, device, cmdQueue, 1280, 720);
    m_depthBufferManager.Initialize(device, 1280, 720);
    m_pipelineManager.Initialize(device, L"assets/VertexShader.cso", L"assets/PixelShader.cso");

    // �摜���[�h
    m_textureManager.Initialize(device);
    m_texIdx = m_textureManager.LoadTexture(L"assets/penguin1.png", m_deviceManager.GetCommandList());
    m_cubeTexIdx= m_textureManager.LoadTexture(L"assets/penguin2.png", m_deviceManager.GetCommandList()); // �ǉ�

    // GameObject�����i�n�ʁ{����Cube�j
    m_gameObjects.clear();

    // --- �n�ʃI�u�W�F�N�g ---
    auto* ground = new GameObject("Ground", 0, m_texIdx, Red);   // �n�ʁFGround.png
    ground->transform.position = XMFLOAT3(0, -1.0f, 0);
    ground->transform.scale = XMFLOAT3(50.0f, 0.2f, 50.0f);
    m_gameObjects.push_back(ground);

    // --- �L���[�u�I�u�W�F�N�g ---
    auto* cube1 = new GameObject("Cube1", 0, m_cubeTexIdx, White); // Cube.png
    cube1->transform.position = XMFLOAT3(-2, 0, 0);
    cube1->transform.scale = XMFLOAT3(1, 1, 1);
    m_gameObjects.push_back(cube1);

    auto* cube2 = new GameObject("Cube2", 0, m_cubeTexIdx, White); // Cube.png
    cube2->transform.position = XMFLOAT3(2, 2, 2);
    cube2->transform.scale = XMFLOAT3(1, 1, 1);
    m_gameObjects.push_back(cube2);

    // ...�ǉ�Cube�����l��...

    // �萔�o�b�t�@�́u256�o�C�g�~�I�u�W�F�N�g���v
    constexpr size_t CBV_SIZE = 256;
    m_bufferManager.CreateConstantBuffer(device, CBV_SIZE * m_gameObjects.size());
    CreateTestCube(); // �����̂̃o�b�t�@�쐬�i�n�ʂ��X�P�[���ŗ��p�j
}

void EngineManager::Start() {
    // �Q�[���J�n���̏����i�g��Ȃ���΋��OK�j
}

void EngineManager::Update() {
    // ���t���[���̍X�V�����i�g��Ȃ���΋��OK�j
}

// ���C���`��֐�
// �G���W���̕`�揈���i�����ŁI�j
void EngineManager::Draw() {
    // 1. �o�b�N�o�b�t�@�E�R�}���h���X�g�擾
    UINT backBufferIndex = m_swapChainManager.GetSwapChain()->GetCurrentBackBufferIndex();
    auto* cmdList = m_deviceManager.GetCommandList();

    // 2. �o���A�ݒ�iPresent��RenderTarget�j
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_swapChainManager.GetBackBuffer(backBufferIndex);
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    cmdList->ResourceBarrier(1, &barrier);

    // 3. RTV/DSV�ݒ�
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_swapChainManager.GetRTVHeap()->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += backBufferIndex * m_swapChainManager.GetRTVHeapSize();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_depthBufferManager.GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
    cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // 4. ��ʃN���A
    const float clearColor[] = { 0.1f, 0.3f, 0.6f, 1.0f };
    cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 5. �r���[�|�[�g�E�V�U�[�ݒ�
    float width = static_cast<float>(m_swapChainManager.GetWidth());
    float height = static_cast<float>(m_swapChainManager.GetHeight());
    D3D12_VIEWPORT viewport = { 0, 0, width, height, 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, (LONG)width, (LONG)height };
    cmdList->RSSetViewports(1, &viewport);
    cmdList->RSSetScissorRects(1, &scissorRect);

    // 6. �p�C�v���C���E���[�g�V�O�l�`���ESRV�q�[�v�ݒ�
    cmdList->SetPipelineState(m_pipelineManager.GetPipelineState());
    cmdList->SetGraphicsRootSignature(m_pipelineManager.GetRootSignature());
    ID3D12DescriptorHeap* heaps[] = { m_textureManager.GetSRVHeap() };
    cmdList->SetDescriptorHeaps(_countof(heaps), heaps);

    // 7. �J�����s��
    static float angle = 0.0f;
    angle += 0.01f;
    XMMATRIX view = XMMatrixLookAtLH(
        XMVectorSet(0, 9, -20, 1), // �J�����ʒu
        XMVectorSet(0, 0, 0, 1),   // �����_
        XMVectorSet(0, 1, 0, 0)    // �����
    );
    XMMATRIX proj = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(60.0f),
        width / height,
        0.1f, 100.0f
    );

    // 8. ��]�l��Transform�ցi�n�ʂ�Cube�������ŉ�]�\�I�j
    for (auto* obj : m_gameObjects) {
        // ��F���ׂẴI�u�W�F�N�g����]������ꍇ
        obj->transform.rotation.y = angle;

         if (obj->name == "Ground") obj->transform.rotation.y = 0;
         else obj->transform.rotation.y = angle; 
    }

    // 9. �萔�o�b�t�@Map�i256�o�C�g�A���C���j
    void* mapped = nullptr;
    HRESULT hr = m_bufferManager.GetConstantBuffer()->Map(0, nullptr, &mapped);
    if (FAILED(hr) || !mapped) return;
    
    // CBV�T�C�Y��256�Œ�
    constexpr size_t CBV_SIZE = 256;

    for (size_t i = 0; i < m_gameObjects.size(); ++i) {
        // 1. �eGameObject�̎擾
        GameObject* obj = m_gameObjects[i];

        // 2. �萔�o�b�t�@�iObjectCB�j������
        ObjectCB cb;
        XMMATRIX world = obj->transform.GetWorldMatrix(); // ���[���h�s����擾

        // �s���GPU�p�ɓ]�u���AView/Proj�������ăZ�b�g
        cb.WorldViewProj = XMMatrixTranspose(world * view * proj);

        // �I�u�W�F�N�g�ŗL�̐F�����Z�b�g
        cb.Color = obj->color;

        // �萔�o�b�t�@�̈�փf�[�^�R�s�[�i�e�I�u�W�F�N�g�����炵�ď������ށj
        memcpy((char*)mapped + CBV_SIZE * i, &cb, sizeof(cb));

        // 3. �萔�o�b�t�@��GPU�A�h���X���v�Z���ă��[�gCBV�ɃZ�b�g
        D3D12_GPU_VIRTUAL_ADDRESS cbvAddr = m_bufferManager.GetConstantBufferGPUAddress() + CBV_SIZE * i;
        cmdList->SetGraphicsRootConstantBufferView(1, cbvAddr); // ���[�g�p�����[�^1�Ƀo�C���h

        // 4. �e�N�X�`�����ݒ肳��Ă���ꍇ�̂�SRV�o�C���h
        if (obj->texIndex >= 0) {
            // �e�N�X�`��SRV�����[�g�p�����[�^0�ɃZ�b�g
            cmdList->SetGraphicsRootDescriptorTable(0, m_textureManager.GetSRV(obj->texIndex));
        }
        // 5. �`��R�}���h�i�v���~�e�B�u�E�o�b�t�@�o�C���h�j
        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // ���_�o�b�t�@�r���[���Z�b�g
        D3D12_VERTEX_BUFFER_VIEW vbv = m_bufferManager.GetVertexBufferView();
        D3D12_INDEX_BUFFER_VIEW ibv = m_bufferManager.GetIndexBufferView();
        cmdList->IASetVertexBuffers(0, 1, &vbv);
        cmdList->IASetIndexBuffer(&ibv);

        // �C���f�b�N�X�h�`��i36���_��12�O�p�`�������̑z��j
        cmdList->DrawIndexedInstanced(36, 1, 0, 0, 0);
    }


    m_bufferManager.GetConstantBuffer()->Unmap(0, nullptr);

    // 11. �o���A��PRESENT�ɖ߂�
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    cmdList->ResourceBarrier(1, &barrier);

    // 12. �R�}���h���X�g���s��Present
    cmdList->Close();
    ID3D12CommandList* commandLists[] = { cmdList };
    m_deviceManager.GetCommandQueue()->ExecuteCommandLists(1, commandLists);
    m_deviceManager.WaitForGpu();
    m_swapChainManager.GetSwapChain()->Present(1, 0);

    // 13. �R�}���h�A���P�[�^/���X�g�����Z�b�g���Ď��t���[������
    m_deviceManager.GetCommandAllocator()->Reset();
    cmdList->Reset(m_deviceManager.GetCommandAllocator(), nullptr);
}


// �I�����̌�n��
// �I�����̌�n��
void EngineManager::Shutdown() {
    // GameObject�̓��I�����������ׂĉ��
    for (auto* obj : m_gameObjects) delete obj;
    m_gameObjects.clear();

    // DX12���\�[�X�̃N���[���A�b�v
    m_deviceManager.Cleanup();
    m_swapChainManager.Cleanup();
    // �K�v�ɉ����đ���Manager�iTextureManager�Ȃǁj��Cleanup()
}


// ���_�E�C���f�b�N�X�o�b�t�@���쐬���A�����̃��f���f�[�^���i�[
void EngineManager::CreateTestCube() {
    std::vector<Vertex> vertices = {
        // �O�ʁi�e�N�X�`����\��ʁj
        { -0.5f, -0.5f, -0.5f, 0, 0 }, // ����
        { -0.5f,  0.5f, -0.5f, 0, 0 }, // ����
        {  0.5f,  0.5f, -0.5f, 0, 0 }, // �E��
        {  0.5f, -0.5f, -0.5f, 0, 0 }, // �E��
        // �E�ʁi�P�F�FUV��0,0�ŌŒ�j
        { 0.5f, -0.5f, -0.5f, 0, 0},
        { 0.5f,  0.5f, -0.5f, 0, 0 },
        { 0.5f,  0.5f,  0.5f, 0, 0 },
        { 0.5f, -0.5f,  0.5f, 0, 0 },
        // ���
        { 0.5f, -0.5f,  0.5f, 0, 0 },
        { 0.5f,  0.5f,  0.5f, 0, 0 },
        { -0.5f,  0.5f,  0.5f, 0, 0 },
        { -0.5f, -0.5f,  0.5f, 0, 0 },
        // ����
        { -0.5f, -0.5f,  0.5f, 0, 0 },
        { -0.5f,  0.5f,  0.5f, 0, 0 },
        { -0.5f,  0.5f, -0.5f, 0, 0 },
        { -0.5f, -0.5f, -0.5f, 0, 0 },
        // ���
        { -0.5f,  0.5f, -0.5f, 0, 1 },
        { -0.5f,  0.5f,  0.5f, 0, 0 },
        { 0.5f,  0.5f,  0.5f, 1, 0 },
        { 0.5f,  0.5f, -0.5f, 1, 1 },
        // ����
        { -0.5f, -0.5f, -0.5f, 0, 0 },
        { -0.5f, -0.5f,  0.5f, 0, 0 },
        { 0.5f, -0.5f,  0.5f, 0, 0 },
        { 0.5f, -0.5f, -0.5f, 0, 0 },
    };
    std::vector<uint16_t> indices = {
        // �e�� 2�O�p�`
        0,1,2, 0,2,3,      // �O��
        4,5,6, 4,6,7,      // �E��
        8,9,10, 8,10,11,   // ���
        12,13,14, 12,14,15,// ����
        16,17,18, 16,18,19,// ���
        20,21,22, 20,22,23 // ����
    };


    m_bufferManager.CreateVertexBuffer(m_deviceManager.GetDevice(), vertices);
    m_bufferManager.CreateIndexBuffer(m_deviceManager.GetDevice(), indices);
}
