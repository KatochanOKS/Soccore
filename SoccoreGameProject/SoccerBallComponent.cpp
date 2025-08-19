#include "SoccerBallComponent.h"
#include "GameObject.h"
#include "Transform.h"
#include <cmath> // for sinf, cosf, fabs

void SoccerBallComponent::Update() {
    auto* tr = gameObject->GetComponent<Transform>();
    tr->position.x += velocity.x;
    tr->position.z += velocity.z;
    // –€ŽC
    velocity.x *= 0.95f;
    velocity.z *= 0.95f;
    if (fabs(velocity.x) < 0.001f) velocity.x = 0;
    if (fabs(velocity.z) < 0.001f) velocity.z = 0;
}

void SoccerBallComponent::Kick(float angleRad, float power) {
    velocity.x += sinf(angleRad) * power;
    velocity.z += cosf(angleRad) * power;
}
