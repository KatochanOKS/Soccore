#pragma once
#include <vector>
#include <type_traits>
#include <string>
#include "Component.h"

// ------------------------------
// �Q�[�����́u�I�u�W�F�N�g�v��\���N���X
// �e��Component�iTransform, Renderer, Collider, etc�j�𕡐������Ƃ��ł���I
// ------------------------------
class GameObject {
public:

    std::string tag;    // ��ޔ��ʗp("Ball", "Goal", "Ground"�Ȃ�)
    std::string name;   // �ʎ��ʗp("Goal1"�Ȃ�)

    // ����GameObject�������ׂĂ�Component�i�|�C���^�j�̃��X�g
    std::vector<Component*> components;

    // ------------------------------
    // �C�ӂ�Component�iTransform, MeshRenderer, �Ȃǁj��ǉ�����ėp�֐�
    // ��: AddComponent<Transform>();
    //     AddComponent<MeshRenderer>();
    // T��Component���p�������^�łȂ��Ƃ����Ȃ�(static_assert�Ő���)
    // ------------------------------
    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value, "T must be a Component");
        T* comp = new T(std::forward<Args>(args)...); // Component�𓮓I����
        comp->gameObject = this; // ������e�Ƃ��ăZ�b�g�iComponent���ŗ��p�\�j
        components.push_back(comp); // ���X�g�ɒǉ�
        return comp;
    }

    // ------------------------------
    // �w�肵���^��Component������Ύ擾�i�Ȃ����nullptr�j
    // ��: Transform* tf = obj->GetComponent<Transform>();
    // ------------------------------
    template<typename T>
    T* GetComponent() {
        for (auto* comp : components) {
            if (auto* casted = dynamic_cast<T*>(comp)) // �^�`�F�b�N����
                return casted; // ����������Ԃ�
        }
        return nullptr; // �Ȃ����nullptr
    }

    // ------------------------------
    // �o�^����Ă���SComponent��Start/Update/Draw���ꊇ�ŌĂ�
    // �i�K�v�ɉ����Ċe�t���[���Ă΂��/�g���j
    // ------------------------------
    void Start() { for (auto* c : components) c->Start(); }
    void Update() { for (auto* c : components) c->Update(); }
    void Draw() { for (auto* c : components) c->Draw(); }

    // ------------------------------
    // �f�X�g���N�^�őSComponent�̃�������delete�i���������[�N�h�~�I�j
    // ------------------------------
    ~GameObject() { for (auto* c : components) delete c; }
};
