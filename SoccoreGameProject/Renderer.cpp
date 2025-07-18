#include "Renderer.h"
using namespace DirectX;

// ������
void Renderer::Initialize(
    DeviceManager* deviceMgr,
    SwapChainManager* swapMgr,
    DepthBufferManager* depthMgr,
    PipelineManager* pipeMgr,
    TextureManager* texMgr,
    BufferManager* cubeBufMgr,
    BufferManager* modelBufMgr,
    BufferManager* cbvBufferMgr,
    FbxModelLoader::VertexInfo* modelVertexInfo
) {
    m_deviceMgr = deviceMgr;
    m_swapMgr = swapMgr;
    m_depthMgr = depthMgr;
    m_pipeMgr = pipeMgr;
    m_texMgr = texMgr;
    m_cubeBufMgr = cubeBufMgr;
    m_modelBufMgr = modelBufMgr;
    m_modelVertexInfo = modelVertexInfo;
    m_cbvBufferMgr = cbvBufferMgr;
    m_width = static_cast<float>(m_swapMgr->GetWidth());
    m_height = static_cast<float>(m_swapMgr->GetHeight());
}

// �t���[���J�n�i�o�b�t�@�N���A�E�^�[�Q�b�g�ݒ�j
void Renderer::BeginFrame() {
    m_backBufferIndex = m_swapMgr->GetSwapChain()->GetCurrentBackBufferIndex();
    m_cmdList = m_deviceMgr->GetCommandList();

    // �o���A�ݒ�iPresent��RenderTarget�j
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_swapMgr->GetBackBuffer(m_backBufferIndex);
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_cmdList->ResourceBarrier(1, &barrier);

    // RTV/DSV�ݒ�
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_swapMgr->GetRTVHeap()->GetCPUDescriptorHandleForHeapStart();
    rtvHandle.ptr += m_backBufferIndex * m_swapMgr->GetRTVHeapSize();
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_depthMgr->GetDSVHeap()->GetCPUDescriptorHandleForHeapStart();
    m_cmdList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // ��ʃN���A
    const float clearColor[] = { 0.1f, 0.3f, 0.6f, 1.0f };
    m_cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_cmdList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // �r���[�|�[�g�E�V�U�[�ݒ�
    D3D12_VIEWPORT viewport = { 0, 0, m_width, m_height, 0.0f, 1.0f };
    D3D12_RECT scissorRect = { 0, 0, (LONG)m_width, (LONG)m_height };
    m_cmdList->RSSetViewports(1, &viewport);
    m_cmdList->RSSetScissorRects(1, &scissorRect);

    // �f�t�H���g�͔�X�L�j���O�p�C�v���C��
    m_cmdList->SetPipelineState(m_pipeMgr->GetPipelineState(false));
    m_cmdList->SetGraphicsRootSignature(m_pipeMgr->GetRootSignature(false));
    ID3D12DescriptorHeap* heaps[] = { m_texMgr->GetSRVHeap() };
    m_cmdList->SetDescriptorHeaps(_countof(heaps), heaps);
}

// �`�揈��
void Renderer::DrawObject(GameObject* obj, size_t idx, const XMMATRIX& view, const XMMATRIX& proj) {
    auto* mr = obj->GetComponent<MeshRenderer>();
    if (!mr) return;

    constexpr size_t CBV_SIZE = 256;
    void* mapped = nullptr;

    // ----------------------
    // �ʏ�L���[�u�E���f���`��
    // ----------------------
    if (mr->meshType == 0 || (mr->meshType == 1 && mr->modelBuffer && mr->vertexInfo)) {
        // �萔�o�b�t�@�ݒ�
        m_cubeBufMgr->GetConstantBuffer()->Map(0, nullptr, &mapped);
        D3D12_GPU_VIRTUAL_ADDRESS cbvAddr = m_cubeBufMgr->GetConstantBufferGPUAddress() + CBV_SIZE * idx;
        m_cmdList->SetGraphicsRootConstantBufferView(1, cbvAddr);

        // SRV�i�e�N�X�`���j
        if (mr->texIndex >= 0)
            m_cmdList->SetGraphicsRootDescriptorTable(0, m_texMgr->GetSRV(mr->texIndex));
        m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // ���f�� or �L���[�u�`��
        if (mr->meshType == 1) {
            D3D12_VERTEX_BUFFER_VIEW vbv = mr->modelBuffer->GetVertexBufferView();
            D3D12_INDEX_BUFFER_VIEW ibv = mr->modelBuffer->GetIndexBufferView();
            m_cmdList->IASetVertexBuffers(0, 1, &vbv);
            m_cmdList->IASetIndexBuffer(&ibv);
            m_cmdList->DrawIndexedInstanced((UINT)mr->vertexInfo->indices.size(), 1, 0, 0, 0);
        }
        else {
            D3D12_VERTEX_BUFFER_VIEW vbv = m_cubeBufMgr->GetVertexBufferView();
            D3D12_INDEX_BUFFER_VIEW ibv = m_cubeBufMgr->GetIndexBufferView();
            m_cmdList->IASetVertexBuffers(0, 1, &vbv);
            m_cmdList->IASetIndexBuffer(&ibv);
            m_cmdList->DrawIndexedInstanced(36, 1, 0, 0, 0);
        }
        m_cubeBufMgr->GetConstantBuffer()->Unmap(0, nullptr);
        return;
    }

    // ----------------------
// --- �X�L�j���OFBX�̏ꍇ ---
    if (mr->meshType == 2 && mr->skinBuffer && mr->skinInfo && mr->boneBuffer) {

        if (mr->skinInfo->vertices.empty() || mr->skinInfo->indices.empty()) {
            OutputDebugStringA("��[SKIN_RENDER] ���_���C���f�b�N�X����ł��I\n");
            return;
        }

        // �X�L�j���O�p�̃p�C�v���C���E���[�g�V�O�l�`�����Z�b�g
        m_cmdList->SetPipelineState(m_pipeMgr->GetPipelineState(true));
        m_cmdList->SetGraphicsRootSignature(m_pipeMgr->GetRootSignature(true));

        // CBV0: ���[���hViewProj
        m_cubeBufMgr->GetConstantBuffer()->Map(0, nullptr, &mapped);
        D3D12_GPU_VIRTUAL_ADDRESS cbvAddr = m_cubeBufMgr->GetConstantBufferGPUAddress() + CBV_SIZE * idx;
        m_cmdList->SetGraphicsRootConstantBufferView(0, cbvAddr);

        // CBV1: �{�[���s��z��
        D3D12_GPU_VIRTUAL_ADDRESS boneCBV = mr->boneBuffer->GetBoneConstantBufferGPUAddress();
        m_cmdList->SetGraphicsRootConstantBufferView(1, boneCBV);

        // �e�N�X�`��SRV�it0�j
        if (mr->texIndex >= 0)
            m_cmdList->SetGraphicsRootDescriptorTable(2, m_texMgr->GetSRV(mr->texIndex));

        // ���_�E�C���f�b�N�X�o�b�t�@
        D3D12_VERTEX_BUFFER_VIEW vbv = mr->skinBuffer->GetVertexBufferView();
        D3D12_INDEX_BUFFER_VIEW ibv = mr->skinBuffer->GetIndexBufferView();
        m_cmdList->IASetVertexBuffers(0, 1, &vbv);
        m_cmdList->IASetIndexBuffer(&ibv);
        m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // �`��
        m_cmdList->DrawIndexedInstanced((UINT)mr->skinInfo->indices.size(), 1, 0, 0, 0);
        m_cubeBufMgr->GetConstantBuffer()->Unmap(0, nullptr);
        return;
    }
}

// �t���[���I���iPresent & �R�}���h���X�g���Z�b�g�j
void Renderer::EndFrame() {
    // �o���A�ݒ�iRenderTarget��Present�j
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Transition.pResource = m_swapMgr->GetBackBuffer(m_backBufferIndex);
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    m_cmdList->ResourceBarrier(1, &barrier);

    // �R�}���h���X�g���s��Present
    m_cmdList->Close();
    ID3D12CommandList* commandLists[] = { m_cmdList };
    m_deviceMgr->GetCommandQueue()->ExecuteCommandLists(1, commandLists);
    m_deviceMgr->WaitForGpu();
    m_swapMgr->GetSwapChain()->Present(1, 0);

    // �R�}���h�A���P�[�^/���X�g�����Z�b�g
    m_deviceMgr->GetCommandAllocator()->Reset();
    m_cmdList->Reset(m_deviceMgr->GetCommandAllocator(), nullptr);
}
