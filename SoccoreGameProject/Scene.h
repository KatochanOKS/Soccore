// Scene.h
#pragma once

// ���ׂẴV�[���̋��ʃC���^�[�t�F�[�X
class Scene {
public:
    virtual ~Scene() {}
    // �������i�ŏ���1�񂾂��Ă΂��j
    virtual void Start() = 0;
    // ���t���[���Ă΂��
    virtual void Update() = 0;
    // ���t���[���Ă΂��i�`�揈���j
    virtual void Draw() = 0;
};
