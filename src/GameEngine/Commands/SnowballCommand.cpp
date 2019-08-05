#include "SnowballCommand.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

SnowballCommand::SnowballCommand(Position pos) : _pos{pos}
{
    _order = 4;  //deviation from their code - but conforms to rules page
}

void SnowballCommand::Execute(bool player1, std::shared_ptr<GameState> state) const
{
    Player* player = state->GetPlayer(player1);
    Player* enemyPlayer = state->GetPlayer(!player1);
    Worm* worm = player->GetCurrentWorm();

    --worm->snowball_count;
    player->consecutiveDoNothingCount = 0;

    if(state->CellType_at(_pos) == CellType::DEEP_SPACE) {
        return; //wasted your bomb...
    }
    
    int points = 0;

    for(auto & thisWorm : enemyPlayer->worms) {
        if(_pos.MaximumDimension(thisWorm.position) <= GameConfig::technologistWorms.snowball.freezeRadius) {
            thisWorm.roundsUntilUnfrozen = GameConfig::technologistWorms.snowball.freezeDuration;
            points += GameConfig::scores.freeze;
        }
    }

    for(auto & thisWorm : player->worms) {
        if(_pos.MaximumDimension(thisWorm.position) <= GameConfig::technologistWorms.snowball.freezeRadius) {
            thisWorm.roundsUntilUnfrozen = GameConfig::technologistWorms.snowball.freezeDuration;
            points -= GameConfig::scores.freeze;
        }
    }

    player->command_score += points;
}

bool SnowballCommand::IsValid(bool player1, std::shared_ptr<GameState> state) const
{
    Player* player = state->GetPlayer(player1);
    Worm* worm = player->GetCurrentWorm();

    if(!_pos.IsOnMap()) {
        std::cerr << latestBot << " (" << __FUNCTION__ << ") snowball target not on map: " << _pos << std::endl;
        return false;
    }

    if(worm->proffession != Worm::Proffession::TECHNOLOGIST) {
        std::cerr << latestBot << " (" << __FUNCTION__ << ") only technologists can throw bananas!" << std::endl;
        return false;
    }

    if(worm->snowball_count <= 0) {
        std::cerr << latestBot << " (" << __FUNCTION__ << ") This guy doesn't have any snowballs left!" << std::endl;
        return false;
    }

    if(!worm->position.BananaSnowballCanReach(_pos)) {
        std::cerr << latestBot << " (" << __FUNCTION__ << ") " << _pos << " is too far to throw a snowball from " << worm->position << "!" << std::endl;
        return false;
    }

    return true; //always valid!
}

bool SnowballCommand::operator==(const SnowballCommand& other)
{
    return _pos == other._pos;
}

std::string SnowballCommand::GetCommandString() const
{
    std::stringstream ret;
    ret << "snowball " << _pos.x << " " << _pos.y;
    return ret.str();
}
