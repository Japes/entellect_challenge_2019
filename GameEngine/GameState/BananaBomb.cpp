#include "BananaBomb.hpp"

bool BananaBomb::operator==(const BananaBomb &other) const
{
    return (damage == other.damage &&
            range == other.range &&
            damageRadius == other.damageRadius);
}
