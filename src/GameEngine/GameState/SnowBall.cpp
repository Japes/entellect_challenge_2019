#include "SnowBall.hpp"

bool SnowBall::operator==(const SnowBall &other) const
{
    return (freezeDuration == other.freezeDuration &&
            range == other.range &&
            freezeRadius == other.freezeRadius);
}
