#include "Player.hpp"

Player::Player(GameState* _state) :
    state{_state},
    id{0},
    command_score{0},
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

Player::Player(const Player& other)
{
    id = other.id;
    command_score = other.command_score;
    health = other.health;
    currentWormId = other.currentWormId;
    consecutiveDoNothingCount = other.consecutiveDoNothingCount;

    for(auto& otherworm : other.worms) {
        worms.push_back(otherworm);
    }
}

Worm* Player::GetCurrentWorm()
{
    unsigned wormIndex = currentWormId - 1;
    if(wormIndex >= worms.size()) {
        throw std::runtime_error("Problem getting next worm for player");
    }
    return &worms[wormIndex];
}

Worm* Player::GetWormById(int id)
{
    return &worms[id - 1];
}

void Player::UpdateCurrentWorm()
{
    if(std::none_of(worms.begin(), worms.end(), [](Worm& w){return !w.IsDead();})) {
        return;
    }

    ++currentWormId;
    if(currentWormId > static_cast<int>(worms.size())) {
        currentWormId = 1;
    }

    while(worms[currentWormId-1].IsDead()) {
        ++currentWormId;
        if(currentWormId > static_cast<int>(worms.size())) {
            currentWormId = 1;
        }
    }
}

void Player::RecalculateHealth()
{
    health = 0;
    std::for_each(worms.begin(), worms.end(), [&](Worm w){ health += (w.health > 0) ? w.health : 0;});
}

int Player::GetAverageWormHealth()
{
    int total_health = 0;
    std::for_each(worms.begin(), worms.end(), [&](Worm w){ total_health += (w.health > 0) ? w.health : 0;});
    return total_health / worms.size();
}

int Player::GetScore()
{
    //std::cerr << "(Player::GetScore) command_score: " << command_score << " GetAverageWormHealth(): " << GetAverageWormHealth() << std::endl;
    return command_score + GetAverageWormHealth();
}

bool Player::operator==(const Player &other) const
{
    bool wormsGood = true;
    for(unsigned i = 0; i < worms.size(); ++i) {
        wormsGood &= worms[i] == other.worms[i];
    }

    //std::cerr << "(" << __FUNCTION__ << ") " <<
    //" wormsGood : " << wormsGood <<
    //" id : " << (id == other.id) <<
    //" command_score : " << (command_score == other.command_score) << " " << command_score << " " << other.command_score <<
    //" health : " << (health == other.health) << " " << health << " " << other.health <<
    //" currentWormId : " << (currentWormId == other.currentWormId) << " " << currentWormId << " " << other.currentWormId <<
    //" consecutiveDoNothingCount : " << (consecutiveDoNothingCount == other.consecutiveDoNothingCount) <<
    //std::endl;

    return wormsGood &&
            id == other.id &&
            command_score == other.command_score &&
            health == other.health &&
            currentWormId == other.currentWormId && //1-indexed
            consecutiveDoNothingCount == other.consecutiveDoNothingCount;
}
