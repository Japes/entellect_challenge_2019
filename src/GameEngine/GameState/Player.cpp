#include "Player.hpp"

Player::Player(GameState* _state) :
    state{_state},
    id{0},
    command_score{0},
    health{0},
    currentWormId{1},
    consecutiveDoNothingCount{0},
    remainingWormSelections{GameConfig::wormSelectTokens},
    worms{Worm(_state), Worm(_state), Worm(_state)}
{
    //not sure why the usual vector constructors aren't working here...
    worms[0].SetProffession(Worm::Proffession::COMMANDO);
    worms[1].SetProffession(Worm::Proffession::AGENT);
    worms[2].SetProffession(Worm::Proffession::TECHNOLOGIST);
    worms[0].id = 1;
    worms[1].id = 2;
    worms[2].id = 3;
    RecalculateHealth();
}

Player::Player(const Player& other) :
    command_score{other.command_score},
    health{other.health},
    currentWormId{other.currentWormId},
    consecutiveDoNothingCount{other.consecutiveDoNothingCount},
    remainingWormSelections{other.remainingWormSelections},
    worms{other.worms},
    previousCommand{other.previousCommand}
{
    SetId(other.id);
}

void Player::SetId(int _id)
{
    id = _id;
    worms[0].playerId = _id;
    worms[1].playerId = _id;
    worms[2].playerId = _id;
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

//have to do this a slightly weird way to be exactly the same as entellect
/*
    fun selectNextWorm() {
        //Assign living worms to a local variable since it is a computed property
        val livingWorms = this.livingWorms
        if (livingWorms.isNotEmpty()) {
            val nextIndex = (livingWorms.indexOf(currentWorm) + 1) % livingWorms.size //indexOf returns -1 if element not in list
            updateCurrentWorm(livingWorms[nextIndex])
        }
    }
*/

void Player::UpdateCurrentWorm()
{
    int livingIndexOfCurrentWorm = -1;
    _livingWorms.clear();
    
    for(auto &worm : worms) {
        if(!worm.IsDead()) {
            if(worm.id == static_cast<unsigned>(currentWormId)) {
                livingIndexOfCurrentWorm = _livingWorms.size();
            }
            _livingWorms.push_back(&worm);
        }
    }

    if(_livingWorms.empty()) {
        return;
    }

    int nextIndex = (livingIndexOfCurrentWorm + 1) % _livingWorms.size();
    currentWormId = _livingWorms[nextIndex]->id;
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

/*
    std::cerr << "(" << __FUNCTION__ << ") " <<
    " wormsGood : " << wormsGood <<
    " id : " << (id == other.id) << " (" << id << ", " << other.id << ")"
    " command_score : " << (command_score == other.command_score) << " " << command_score << " " << other.command_score <<
    " health : " << (health == other.health) << " " << health << " " << other.health <<
    " currentWormId : " << (currentWormId == other.currentWormId) << " " << currentWormId << " " << other.currentWormId <<
    " consecutiveDoNothingCount : " << (consecutiveDoNothingCount == other.consecutiveDoNothingCount) << " (" << consecutiveDoNothingCount << ", " << other.consecutiveDoNothingCount << ")" <<
    std::endl;
    */

    return wormsGood &&
            id == other.id &&
            command_score == other.command_score &&
            health == other.health &&
            //this isn't in the state files, shouldn't make a difference (this operator is only for unit tests)
            //consecutiveDoNothingCount == other.consecutiveDoNothingCount && 
            remainingWormSelections == other.remainingWormSelections &&
            currentWormId == other.currentWormId; //1-indexed
}
