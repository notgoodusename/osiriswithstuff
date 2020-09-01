#pragma once

#include "../SDK/Entity.h"
#include "../SDK/Vector.h"

struct UserCmd;
struct Vector;

namespace Aimbot
{
    Vector calculateRelativeAngle(const Vector& source, const Vector& destination, const Vector& viewAngles) noexcept;
    Vector velocityExtrapolate(Entity* entity, const Vector& destination) noexcept;
}
