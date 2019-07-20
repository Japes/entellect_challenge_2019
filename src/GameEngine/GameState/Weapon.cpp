#include "Weapon.hpp"

bool Weapon::operator==(const Weapon &other) const
{
    return (damage == other.damage &&
            range == other.range &&
            diagRange == other.diagRange);
}
