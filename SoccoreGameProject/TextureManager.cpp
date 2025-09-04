#include "TextureManager.h"
#include <DirectXTex.h>   // WIC���[�_ / CreateTexture / UpdateSubresources �ɕK�v
#include <cassert>
#include "d3dx12.h"

void TextureManager::Initialize(ID3D12Device* device, UINT maxTextureNum) {
    // 1) SRV�p�̃f�B�X�N���v�^�q�[�v���쐬�i�V�F�[�_���猩����悤�� SHADER_VISIBLE�j
    m_device = device;
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = maxTextureNum;                               // �\��
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;               // SRV/CBV/UAV�p
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;           // ����厖�F�V�F�[�_����g��

    // CreateDescriptorHeap �Ɏ��s����ƒv���I�Bhr�`�F�b�N�������ɂ���Ȃ� assert �̑���� if(FAILED(hr)) �Ń��O��return�B
    HRESULT hr = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_srvHeap));
    assert(SUCCEEDED(hr));

    // 2) SRV�f�B�X�N���v�^�́u1������̃T�C�Y�v��₢���킹�Ă����i�I�t�Z�b�g�v�Z�Ɏg���j
    m_descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // 3) ���Ɏg���󂫃X���b�g�ԍ���0�Ƀ��Z�b�g
    m_nextIndex = 0;
}

int TextureManager::LoadTexture(const std::wstring& filename, ID3D12GraphicsCommandList* cmdList) {
    // 0) ��d�ǂݍ��ݑ΍�F�����p�X�͓����C���f�b�N�X��Ԃ�
    auto it = m_textureIndices.find(filename);
    if (it != m_textureIndices.end()) return it->second;

    // 1) CPU���։摜���[�h�iWIC�Fpng/jpg/bmp���j
    DirectX::TexMetadata metadata = {};
    DirectX::ScratchImage image = {};
    // �����FsRGB�摜�Ȃ� WIC_FLAGS_FORCE_SRGB ���g���Ɣ��F���������Ȃ�ꍇ������
    HRESULT hr = DirectX::LoadFromWICFile(
        filename.c_str(),
        DirectX::WIC_FLAGS_NONE, // ������ WIC_FLAGS_FORCE_SRGB �� WIC_FLAGS_IGNORE_SRGB �ɕς���ƐF��Ԃ𐧌�ł���
        &metadata,
        image
    );
    assert(SUCCEEDED(hr)); // ���s�Ȃ�t�@�C���p�X/�g���q/���΃p�X���m�F

    // 2) GPU(Default heap)���̃e�N�X�`�����\�[�X���쐬�i�܂����g�͋�j
    Microsoft::WRL::ComPtr<ID3D12Resource> texture;
    hr = DirectX::CreateTexture(m_device.Get(), metadata, &texture);
    assert(SUCCEEDED(hr));

    // 3) Upload heap �̈ꎞ�o�b�t�@��p�Ӂi�R�s�[���j
    //    �摜�̑S�T�u���\�[�X�i�~�b�v/�z��/�ʁj��]������̂ɕK�v�ȃT�C�Y�����ς���
    const UINT subresourceCount = static_cast<UINT>(image.GetImageCount());
    size_t uploadBufferSize = GetRequiredIntermediateSize(texture.Get(), 0, subresourceCount);

    Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
    CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);           // CPU�������݉�
    CD3DX12_RESOURCE_DESC   resDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadBufferSize);
    hr = m_device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, // Upload�͏�ɂ��̃X�e�[�g
        nullptr,
        IID_PPV_ARGS(&uploadBuffer)
    );
    assert(SUCCEEDED(hr));

    // 4) UpdateSubresources �� Upload �� Default �փR�s�[���邽�߂̍\���̔z����쐬
    std::vector<D3D12_SUBRESOURCE_DATA> subresources;
    subresources.reserve(subresourceCount);
    const DirectX::Image* img = image.GetImages();
    for (size_t i = 0; i < image.GetImageCount(); ++i) {
        D3D12_SUBRESOURCE_DATA sub = {};
        sub.pData = img[i].pixels;     // �s�N�Z���擪
        sub.RowPitch = img[i].rowPitch;   // 1�s������̃o�C�g��
        sub.SlicePitch = img[i].slicePitch; // 1�ʁi�~�b�v/�A���C�v�f�j�̃o�C�g��
        subresources.push_back(sub);
    }

    // 5) �R�s�[�iCopyDest �� PixelShaderResource �֑J�ځj
    //    UpdateSubresources �͓����� CopyBufferRegion / CopyTextureRegion ���L�^���Ă����
    UpdateSubresources(cmdList, texture.Get(), uploadBuffer.Get(), 0, 0, subresourceCount, subresources.data());

    // 6) �V�F�[�_����Q�Ƃł���悤�ɃX�e�[�g�J�ځiCOPY_DEST �� PIXEL_SHADER_RESOURCE�j
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        texture.Get(),
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
    );
    cmdList->ResourceBarrier(1, &barrier);

    // 7) SRV�iShader Resource View�j���A�q�[�v�� m_nextIndex �Ԃɍ쐬
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // �ʏ�͊���}�b�s���O
    srvDesc.Format = metadata.format;                      // �摜�t�H�[�}�b�g�isRGB�ɂ������Ȃ�ϊ��������j
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // �����2D����̑z��
    srvDesc.Texture2D.MipLevels = static_cast<UINT>(metadata.mipLevels); // WIC�ǂݍ��݂͒ʏ�~�b�v1���B�K�v�Ȃ玩�O�Ń~�b�v����

    // 7-1) �q�[�v�擪CPU�n���h�� + �I�t�Z�b�g�i�C���N�������g���~�C���f�b�N�X�j
    D3D12_CPU_DESCRIPTOR_HANDLE handle = m_srvHeap->GetCPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<SIZE_T>(m_nextIndex) * m_descriptorSize;

    // 7-2) SRV�����i���̎��_�Ńe�N�X�`���� PS ����Q�Ɖ\�j
    m_device->CreateShaderResourceView(texture.Get(), &srvDesc, handle);

    // 8) �Ǘ��e�[�u���ɓo�^
    m_textures.push_back(texture);                // ���́iDefault�j�ێ�
    const int texIndex = static_cast<int>(m_nextIndex);
    m_textureIndices[filename] = texIndex;        // �p�X���C���f�b�N�X�̎���
    m_nextIndex++;                                // ���̋󂫃X���b�g��
    m_uploadBuffers.push_back(uploadBuffer);      // ���d�v�FGPU�]�������܂ŉ�����Ȃ����ߕێ�

    // 9) �G���[���O�ihr �͒��O��CreateSRV�̖߂�ł͂Ȃ��̂Œ��ӁB�K�v�Ȃ�ʃ`�F�b�N�ɂ���j
    if (FAILED(hr)) {
        OutputDebugStringA("���e�N�X�`���ǂݍ��ݎ��s�I\n");
    }

    // 10) SRV�X���b�g�ԍ���Ԃ��i�`�摤�� GetSRV(texIndex) �� GPU�n���h���𓾂�j
    return texIndex;
}

ID3D12DescriptorHeap* TextureManager::GetSRVHeap() {
    // �`��O�� CommandList::SetDescriptorHeaps �ł��̃q�[�v���Z�b�g���邽�߂ɕԂ�
    return m_srvHeap.Get();
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSRV(int index) {
    // SRV�e�[�u���́uGPU���v�擪���� index �������|�C���^��i�߂��n���h����Ԃ�
    D3D12_GPU_DESCRIPTOR_HANDLE handle = m_srvHeap->GetGPUDescriptorHandleForHeapStart();
    handle.ptr += static_cast<UINT64>(index) * m_descriptorSize;
    return handle;
}
