#pragma once
#include <vector>
#include <type_traits>
#include <string>
#include "Component.h"

// ------------------------------
// ゲーム内の「オブジェクト」を表すクラス
// 各種Component（Transform, Renderer, Collider, etc）を複数持つことができる！
// ------------------------------
class GameObject {
public:

    std::string tag;    // 種類判別用("Ball", "Goal", "Ground"など)
    std::string name;   // 個別識別用("Goal1"など)

    // このGameObjectが持つすべてのComponent（ポインタ）のリスト
    std::vector<Component*> components;

    // ------------------------------
    // 任意のComponent（Transform, MeshRenderer, など）を追加する汎用関数
    // 例: AddComponent<Transform>();
    //     AddComponent<MeshRenderer>();
    // TはComponentを継承した型でないといけない(static_assertで制限)
    // ------------------------------
    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        static_assert(std::is_base_of<Component, T>::value, "T must be a Component");
        T* comp = new T(std::forward<Args>(args)...); // Componentを動的生成
        comp->gameObject = this; // 自分を親としてセット（Component側で利用可能）
        components.push_back(comp); // リストに追加
        return comp;
    }

    // ------------------------------
    // 指定した型のComponentがあれば取得（なければnullptr）
    // 例: Transform* tf = obj->GetComponent<Transform>();
    // ------------------------------
    template<typename T>
    T* GetComponent() {
        for (auto* comp : components) {
            if (auto* casted = dynamic_cast<T*>(comp)) // 型チェックして
                return casted; // 発見したら返す
        }
        return nullptr; // なければnullptr
    }

    // ------------------------------
    // 登録されている全ComponentのStart/Update/Drawを一括で呼ぶ
    // （必要に応じて各フレーム呼ばれる/拡張可）
    // ------------------------------
    void Start() { for (auto* c : components) c->Start(); }
    void Update() { for (auto* c : components) c->Update(); }
    void Draw() { for (auto* c : components) c->Draw(); }

    // ------------------------------
    // デストラクタで全Componentのメモリをdelete（メモリリーク防止！）
    // ------------------------------
    ~GameObject() { for (auto* c : components) delete c; }
};
