#include "Renderer.h"
#include "d3dx12.h"
#include "Transform.h"
#include "StaticMeshRenderer.h"   // �� �ǉ��I
#include "SkinnedMeshRenderer.h"  // �� �ǉ��I

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
    m_width = static_cast<float>(m_swapMgr->GetWidth());
    m_height = static_cast<float>(m_swapMgr->GetHeight());

    // --- �X�L�j���O�pCBV�o�b�t�@�쐬�i80�{�[���z��j
    m_skinCBSize = sizeof(DirectX::XMMATRIX) * 80; // �K�v�ɉ����Ē���
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC cbDesc = CD3DX12_RESOURCE_DESC::Buffer((m_skinCBSize + 255) & ~255); // 256�A���C��

    m_deviceMgr->GetDevice()->CreateCommittedResource(
        &heapProps, D3D12_HEAP_FLAG_NONE, &cbDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        IID_PPV_ARGS(&m_skinningConstantBuffer)
    );
    m_skinCBGpuAddr = m_skinningConstantBuffer->GetGPUVirtualAddress();
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

// �`�揈���i�^���Ƃɕ���I�j
void Renderer::DrawObject(GameObject* obj, size_t idx, const XMMATRIX& view, const XMMATRIX& proj) {
    constexpr size_t CBV_SIZE = 256;

    // 1. �X�L�j���O���b�V���i�A�j���t�����f���j
    if (auto* smr = obj->GetComponent<SkinnedMeshRenderer>()) {
        m_cmdList->SetPipelineState(m_pipeMgr->GetPipelineState(true));
        m_cmdList->SetGraphicsRootSignature(m_pipeMgr->GetRootSignature(true));

        if (smr->texIndex >= 0)
            m_cmdList->SetGraphicsRootDescriptorTable(0, m_texMgr->GetSRV(smr->texIndex));

        XMMATRIX world = obj->GetComponent<Transform>()->GetWorldMatrix();
        XMMATRIX wvp = XMMatrixTranspose(world * view * proj);

        // �X�L�j���O�s��擾�i�A�j���[�^�[�K�{�I�j
        std::vector<XMMATRIX> skinnedMats;
        if (smr->animator && smr->skinVertexInfo) {
            skinnedMats = smr->animator->GetSkinnedPose(smr->skinVertexInfo->bindPoses);
            for (auto& m : skinnedMats)
                m = XMMatrixTranspose(m);
        }

        // �{�[���s��{WVP��萔�o�b�t�@�ɏ�������
        void* mapped = nullptr;
        if (SUCCEEDED(m_skinningConstantBuffer->Map(0, nullptr, &mapped))) {
            memcpy((char*)mapped, &wvp, sizeof(XMMATRIX)); // b0: WVP
            if (!skinnedMats.empty())
                memcpy((char*)mapped + 256, skinnedMats.data(), sizeof(XMMATRIX) * skinnedMats.size()); // b1: Bone array
            m_skinningConstantBuffer->Unmap(0, nullptr);

            m_cmdList->SetGraphicsRootConstantBufferView(1, m_skinCBGpuAddr);           // b0
            m_cmdList->SetGraphicsRootConstantBufferView(2, m_skinCBGpuAddr + 256);     // b1
        }

        if (smr->modelBuffer && smr->skinVertexInfo) {
            auto vbv = smr->modelBuffer->GetVertexBufferView();
            auto ibv = smr->modelBuffer->GetIndexBufferView();
            m_cmdList->IASetVertexBuffers(0, 1, &vbv);
            m_cmdList->IASetIndexBuffer(&ibv);
            m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            m_cmdList->DrawIndexedInstanced((UINT)smr->skinVertexInfo->indices.size(), 1, 0, 0, 0);
        }
        return;
    }

    // 2. �ÓI���b�V���iCube��ʏ�FBX�j
    if (auto* mr = obj->GetComponent<StaticMeshRenderer>()) {
        D3D12_GPU_VIRTUAL_ADDRESS cbvAddr = m_cubeBufMgr->GetConstantBufferGPUAddress() + CBV_SIZE * idx;
        m_cmdList->SetGraphicsRootConstantBufferView(1, cbvAddr);

        if (mr->texIndex >= 0)
            m_cmdList->SetGraphicsRootDescriptorTable(0, m_texMgr->GetSRV(mr->texIndex));
        m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        // FBX���f���̏ꍇ
        if (mr->modelBuffer && mr->vertexInfo) {
            auto vbv = mr->modelBuffer->GetVertexBufferView();
            auto ibv = mr->modelBuffer->GetIndexBufferView();
            m_cmdList->IASetVertexBuffers(0, 1, &vbv);
            m_cmdList->IASetIndexBuffer(&ibv);
            m_cmdList->DrawIndexedInstanced((UINT)mr->vertexInfo->indices.size(), 1, 0, 0, 0);
        }
        // Cube�Ȃǂ̋��ʃo�b�t�@�̏ꍇ
        else {
            auto vbv = m_cubeBufMgr->GetVertexBufferView();
            auto ibv = m_cubeBufMgr->GetIndexBufferView();
            m_cmdList->IASetVertexBuffers(0, 1, &vbv);
            m_cmdList->IASetIndexBuffer(&ibv);
            m_cmdList->DrawIndexedInstanced(36, 1, 0, 0, 0);
        }
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
