#include "Player.hpp"

Player::Player(GameState* _state) :
    state{_state},
    id{0},
    score{0},
    health{0},
    currentWormId{1},
    consecutiveDoNothingCount{0}
{
    //not sure why the usual vector constructors aren't working here...
    worms.push_back(Worm(state));
    worms.push_back(Worm(state));
    worms.push_back(Worm(state));
    RecalculateHealth();
}

Worm* Player::GetCurrentWorm()
{
    unsigned wormIndex = currentWormId - 1;
    if(wormIndex >= worms.size()) {
        throw std::runtime_error("Problem getting next worm for player");
    }
    return &worms[wormIndex];
}

Worm* Player::GetWormByIndex(int index)
{
    return &worms[index - 1];
}

void Player::UpdateCurrentWorm()
{
    ++currentWormId;
    if(currentWormId > static_cast<int>(worms.size())) {
        currentWormId = 1;
    }
}

void Player::RecalculateHealth()
{
    health = 0;
    std::for_each(worms.begin(), worms.end(), [&](Worm w){ health += (w.health > 0) ? w.health : 0;});
}
