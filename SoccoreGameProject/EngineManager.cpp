#include "EngineManager.h"
#include "BufferManager.h" // Vertex���g������
#include <DirectXMath.h>
using namespace DirectX;

// �G���W���̏���������
void EngineManager::Initialize() {
    // DeviceManager�̏������iGPU�f�o�C�X�E�R�}���h����j
    m_deviceManager.Initialize();

    // SwapChainManager�̏������i�E�B���h�E�E�o�b�N�o�b�t�@�Ǘ��j
    auto* device = m_deviceManager.GetDevice();
    auto* cmdQueue = m_deviceManager.GetCommandQueue();
    m_swapChainManager.Initialize(m_hWnd, device, cmdQueue, 1280, 720);

    // �f�v�X�o�b�t�@�i���s���e�X�g�p�j
    m_depthBufferManager.Initialize(device, 1280, 720);

    // �p�C�v���C���E�V�F�[�_������
    m_pipelineManager.Initialize(device, L"assets/VertexShader.cso", L"assets/PixelShader.cso");

    // �萔�o�b�t�@�iCBV�j���u4���v�܂Ƃ߂Ċm�ہi�L���[�u4���̍s��j
    m_bufferManager.CreateConstantBuffer(m_deviceManager.GetDevice(), sizeof(XMMATRIX) * 4);

    // �R�}���h���X�g�擾�i���\�[�X�쐬�p�j
    auto* cmdList = m_deviceManager.GetCommandList();

    // �e�N�X�`���}�l�[�W�����������摜���[�h�ipenguin1.png��SRV�Ƃ��ăZ�b�g�j
    m_textureManager.Initialize(device);
    m_texIdx = m_textureManager.LoadTexture(L"assets/penguin1.png", m_deviceManager.GetCommandList());

    // --- ��������V�[���ɔz�u����L���[�u��ݒ� ---
    m_cubeTransforms.clear();

    // �L���[�u1
    Transform t1;
    t1.position = XMFLOAT3(-2.0f, 0.0f, 0.0f); // ��
    t1.scale = XMFLOAT3(1.0f, 1.0f, 1.0f);     // �W���T�C�Y
    m_cubeTransforms.push_back(t1);

    // �L���[�u2
    Transform t2;
    t2.position = XMFLOAT3(0.0f, 5.0f, 0.0f);  // ��ɂ���
    t2.scale = XMFLOAT3(1.0f, 1.0f, 1.0f);     // �W���T�C�Y
    m_cubeTransforms.push_back(t2);

    // �L���[�u3
    Transform t3;
    t3.position = XMFLOAT3(2.0f, 2.0f, 2.0f);  // �E��
    t3.scale = XMFLOAT3(1.0f, 1.0f, 1.0f);     // �W���T�C�Y
    m_cubeTransforms.push_back(t3);

    // �L���[�u4
    Transform t4;
    t4.position = XMFLOAT3(-5.0f, 6.0f, 0.0f); // ����
    t4.scale = XMFLOAT3(2.0f, 2.0f, 2.0f);     // �傫��
    m_cubeTransforms.push_back(t4);

    // ���_�E�C���f�b�N�X�o�b�t�@�����i������1���̂ݍ���OK�j
    CreateTestCube();
}

void EngineManager::Start() {
    // �Q�[���J�n���̏����i�g��Ȃ���΋��OK�j
}

void EngineManager::Update() {
    // ���t���[���̍X�V�����i�g��Ȃ���΋��OK�j
}

// ���C���`��֐�
void EngineManager::Draw() {
    // 1. ���`�悷��o�b�N�o�b�t�@�ԍ��擾�A�R�}���h���X�g�擾
    UINT backBufferIndex = m_swapChainManager.GetSwapChain()->GetCurrentBackBufferIndex();
    auto* cmdList = m_deviceManager.GetCommandList();

    // 2. �o�b�N�o�b�t�@�̏�Ԃ��u�`��\�v�ɐ؂�ւ���o���A�ݒ�
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_swapChainManager.GetBackBuffer(backBufferIndex);
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    cmdList->ResourceBarrier(1, &barrier);

    // 3. RTV�i�J���[�o�b�t�@�j�EDSV�i�[�x�o�b�t�@�j�̃n���h���擾
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_swapChainManager.GetRTVHeap()->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += backBufferIndex * m_swapChainManager.GetRTVHeapSize();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_depthBufferManager.GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();

    // 4. �����_�[�^�[�Q�b�g�iRTV/DSV�j��ݒ�
    cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // 5. ��ʃN���A�i�F�A���s�����N���A�j
    const float clearColor[] = { 0.1f, 0.3f, 0.6f, 1.0f };
    cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // 6. �r���[�|�[�g�E�V�U�[�ݒ�i��ʑS�́j
    float width = static_cast<float>(m_swapChainManager.GetWidth());
    float height = static_cast<float>(m_swapChainManager.GetHeight());
    D3D12_VIEWPORT viewport = { 0, 0, width, height, 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, (LONG)width, (LONG)height };
    cmdList->RSSetViewports(1, &viewport);
    cmdList->RSSetScissorRects(1, &scissorRect);

    // 7. �p�C�v���C���X�e�[�g�E���[�g�V�O�l�`���ݒ�ifor���[�v�̊O��1�񂾂��I�j
    cmdList->SetPipelineState(m_pipelineManager.GetPipelineState());
    cmdList->SetGraphicsRootSignature(m_pipelineManager.GetRootSignature());

    // 8. �e�N�X�`���iSRV�j���V�F�[�_�Ƀo�C���h
    ID3D12DescriptorHeap* heaps[] = { m_textureManager.GetSRVHeap() };
    cmdList->SetDescriptorHeaps(_countof(heaps), heaps);
    cmdList->SetGraphicsRootDescriptorTable(0, m_textureManager.GetSRV(m_texIdx));

    // 9. �J�����̍s��i�r���[�E�v���W�F�N�V�����j
    static float angle = 0.0f;
    angle += 0.01f; // ��ʑS�̂���]�����邽�߂̊p�x
    XMMATRIX view = XMMatrixLookAtLH(
        XMVectorSet(0, 10, -10, 1),  // �J�����ʒu
        XMVectorSet(0, 0, 0, 1),    // �����_
        XMVectorSet(0, 1, 0, 0)     // �����
    );
    XMMATRIX proj = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(60.0f),
        width / height,
        0.1f, 100.0f
    );

    // 10. �萔�o�b�t�@��Map�ifor�̊O�ň�x�����j
    void* mapped = nullptr;
    HRESULT hr = m_bufferManager.GetConstantBuffer()->Map(0, nullptr, &mapped);
    if (FAILED(hr) || mapped == nullptr) return;

    // 11. ���ׂẴL���[�u��for���ŕ`��
    for (int i = 0; i < m_cubeTransforms.size(); ++i) {
        const auto& t = m_cubeTransforms[i];
        // ���[���h�s��i�S�̂�Y����]������j
        XMMATRIX world = XMMatrixRotationY(angle) * t.GetWorldMatrix();
        XMMATRIX wvp = XMMatrixTranspose(world * view * proj);

        // CBV�o�b�t�@���̎����̗̈�֍s�����������
        memcpy((char*)mapped + sizeof(XMMATRIX) * i, &wvp, sizeof(wvp));

        // �����p�̒萔�o�b�t�@�A�h���X���o�C���h
        D3D12_GPU_VIRTUAL_ADDRESS cbvAddress = m_bufferManager.GetConstantBufferGPUAddress() + sizeof(XMMATRIX) * i;
        cmdList->SetGraphicsRootConstantBufferView(1, cbvAddress);

        // �o�b�t�@�o�C���h�E�`��R�}���h���s
        cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        D3D12_VERTEX_BUFFER_VIEW vbv = m_bufferManager.GetVertexBufferView();
        D3D12_INDEX_BUFFER_VIEW ibv = m_bufferManager.GetIndexBufferView();
        cmdList->IASetVertexBuffers(0, 1, &vbv);
        cmdList->IASetIndexBuffer(&ibv);
        cmdList->DrawIndexedInstanced(36, 1, 0, 0, 0);
    }
    // �萔�o�b�t�@�A���}�b�v
    m_bufferManager.GetConstantBuffer()->Unmap(0, nullptr);

    // 12. �o���A��Present�p�ɐ؂�ւ�
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    cmdList->ResourceBarrier(1, &barrier);

    // 13. �R�}���h���X�g��GPU�Ɏ��s������
    cmdList->Close();
    ID3D12CommandList* commandLists[] = { cmdList };
    m_deviceManager.GetCommandQueue()->ExecuteCommandLists(1, commandLists);
    m_deviceManager.WaitForGpu();

    // 14. ��ʂ�\���i�X���b�v�`�F�C����Present�j
    m_swapChainManager.GetSwapChain()->Present(1, 0);

    // 15. �R�}���h�A���P�[�^/���X�g�����Z�b�g���Ď��̃t���[������
    m_deviceManager.GetCommandAllocator()->Reset();
    cmdList->Reset(m_deviceManager.GetCommandAllocator(), nullptr);
}

// �I�����̌�n��
void EngineManager::Shutdown() {
    m_deviceManager.Cleanup();
    m_swapChainManager.Cleanup();
}

// ���_�E�C���f�b�N�X�o�b�t�@���쐬���A�����̃��f���f�[�^���i�[
void EngineManager::CreateTestCube() {
    std::vector<Vertex> vertices = {
        // �O�ʁi�e�N�X�`����\��ʁj
        { -0.5f, -0.5f, -0.5f, 0, 0.5f }, // ����
        { -0.5f,  0.5f, -0.5f, 0, 0 }, // ����
        {  0.5f,  0.5f, -0.5f, 0.5f, 0 }, // �E��
        {  0.5f, -0.5f, -0.5f, 0.5f, 0.5f }, // �E��
        // �E�ʁi�P�F�FUV��0,0�ŌŒ�j
        { 0.5f, -0.5f, -0.5f, 0, 1 },
        { 0.5f,  0.5f, -0.5f, 0, 0 },
        { 0.5f,  0.5f,  0.5f, 1, 0 },
        { 0.5f, -0.5f,  0.5f, 1, 1 },
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
        { -0.5f,  0.5f, -0.5f, 0, 0 },
        { -0.5f,  0.5f,  0.5f, 0, 0 },
        { 0.5f,  0.5f,  0.5f, 0, 0 },
        { 0.5f,  0.5f, -0.5f, 0, 0 },
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
