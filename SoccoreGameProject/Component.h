#pragma once
class GameObject;

class Component {
public:
    GameObject* gameObject = nullptr;

    virtual ~Component() {}
    virtual void Start() {}
    virtual void Update() {}
    virtual void Draw() {}
};
