#pragma once
#include "Types.h"

constexpr Vec3 ApplyGravity(const Vec3 velocity, const float gravity_scale,
                         const float dt) {
    return velocity + Vec3{0.0f, -9.81f * gravity_scale, 0.0f} * dt;
}

constexpr Vec3 ApplyDrag(const Vec3 velocity, const float drag_coefficient,
                      const float dt) {
    return velocity * (1.0f - drag_coefficient) * dt;
}

constexpr Vec3 IntegrateMovement(const Vec3 position, const Vec3 velocity,
                                 const float dt) {
    return position + velocity * dt;
}

constexpr bool LifecycleCheck(const float age, const float lifetime) {
    return age >= lifetime;
}
