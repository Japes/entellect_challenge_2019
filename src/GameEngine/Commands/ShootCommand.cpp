#include "ShootCommand.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

const Position ShootCommand::noShot{0,0};

ShootCommand::ShootCommand(ShootCommand::ShootDirection dir)
{
    _order = static_cast<int>(CommandType::SHOOT);
    switch (dir)
    {
        case ShootDirection::N:     _shootVector = Position(0,-1); break;
        case ShootDirection::NE:    _shootVector = Position(1,-1); break;
        case ShootDirection::E:     _shootVector = Position(1,0); break;
        case ShootDirection::SE:    _shootVector = Position(1,1); break;
        case ShootDirection::S:     _shootVector = Position(0,1); break;
        case ShootDirection::SW:    _shootVector = Position(-1,1); break;
        case ShootDirection::W:     _shootVector = Position(-1,0); break;
        case ShootDirection::NW:    _shootVector = Position(-1,-1); break;
    }
}

ShootCommand::ShootCommand(Position dir)
{
    _order = static_cast<int>(CommandType::SHOOT);

    //normalise
    if(dir.x > 0) {
        dir.x = 1;
    } else if(dir.x < 0) {
        dir.x = -1;
    }

    if(dir.y > 0) {
        dir.y = 1;
    } else if(dir.y < 0) {
        dir.y = -1;
    }

    _shootVector = dir;
}

void ShootCommand::Execute(bool player1, GameStatePtr state) const
{
    Player* player = state->GetPlayer(player1);
    Worm* worm = player->GetCurrentWorm();
    if(worm->IsFrozen()) {
        return;
    }

    player->consecutiveDoNothingCount = 0;

    Worm* hitworm = WormOnTarget(worm, state, _shootVector);

    if(hitworm == nullptr) {
        player->command_score += GameConfig::scores.missedAttack;
        return;
    }

    hitworm->TakeDamage(worm->weapon.damage);

    int points = worm->weapon.damage*2;
    
    if(hitworm->IsDead()) {
        points += GameConfig::scores.killShot;
    }

    if(std::any_of(player->worms.begin(), player->worms.end(), [&](Worm& w){return &w == hitworm;})) {
        points *= -1;
    }

    player->command_score += points;
}

//returns worm that would be hit if worm shot in direction of shootvector
Worm* ShootCommand::WormOnTarget(const Worm* worm, const GameStatePtr state, const Position& shootvector)
{
    Position pos = worm->position + shootvector;

    bool diag = shootvector.x != 0 && shootvector.y != 0;

    while (pos.IsOnMap() && worm->position.MovementDistanceTo(pos) <= worm->weapon.range) {

        //check diag
        if( diag && worm->position.MovementDistanceTo(pos) > worm->weapon.diagRange) {
            //std::cerr << "OUT OF RANGE! (" <<worm->position.MovementDistanceTo(pos) << " > " << worm->weapon.diagRange << ")" << std::endl;
            return nullptr;
        }

        //check dirt/deep space
        if (IsBlocking(state->CellType_at(pos))) {
            //std::cerr << "HIT DIRT (or deep space)!" << std::endl;
            return nullptr;
        }

        Worm* w = state->Worm_at(pos);
        if (w != nullptr) {
            //std::cerr << "HIT A WORM!" << std::endl;
            return w;
        }

        pos += shootvector;
    }

    //if we get this far, shot didn't hit anything
    return nullptr;
}

bool ShootCommand::IsValid(bool player1, GameStatePtr state) const
{
    return true; //always valid!
}

//returns a vector pointing in the direction of a clear shot to target
//returns {0,0} if no shot available
Position ShootCommand::GetValidShot(const Worm& shootingWorm, const Position& target, GameStatePtr state)
{
    if(shootingWorm.position.MovementDistanceTo(target) > shootingWorm.weapon.range) {
        return {0,0};
    }

    Position posDiff = target - shootingWorm.position;

    bool isStraight = posDiff.x == 0 || posDiff.y == 0;
    bool isInLine = ( isStraight || posDiff.x == posDiff.y || posDiff.x == -posDiff.y);
    if( !isInLine) {
        return {0,0};
    }

    if(!isStraight && shootingWorm.position.MovementDistanceTo(shootingWorm.position) > shootingWorm.weapon.diagRange) {
        return {0,0};
    }

    //finally, check for obstacles
    Position shootVec = posDiff.Normalized();
    if (!ClearShot(&shootingWorm, state, shootVec, target)) {
        return {0,0};
    }

    return shootVec;
}

//returns true if worm can shoot the target position
bool ShootCommand::ClearShot(const Worm* worm, const GameStatePtr state, const Position& shootvector, const Position& targetPos)
{
    Position pos = worm->position + shootvector;

    bool diag = shootvector.x != 0 && shootvector.y != 0;

    while (pos.IsOnMap() && worm->position.MovementDistanceTo(pos) <= worm->weapon.range) {

        //check diag
        if( diag && worm->position.MovementDistanceTo(pos) > worm->weapon.diagRange) {
            //std::cerr << "OUT OF RANGE! (" <<worm->position.MovementDistanceTo(pos) << " > " << worm->weapon.diagRange << ")" << std::endl;
            return false;
        }

        //check dirt/deep space
        if (IsBlocking(state->CellType_at(pos))) {
            //std::cerr << "HIT DIRT (or deep space)!" << std::endl;
            return false;
        }

        if (pos == targetPos) {
            //std::cerr << "I CAN SHOOT HERE!" << std::endl;
            return true;
        }

        //check worms 
        Worm* w = state->Worm_at(pos);
        if (w != nullptr) {
            //std::cerr << "WORM IN THE WAY!" << std::endl;
            return false;
        }

        pos += shootvector;
    }

    //if we get this far, shot didn't hit anything
    return false;
}


bool ShootCommand::operator==(const ShootCommand& other)
{
        return
        _shootVector == other._shootVector;
}

std::string ShootCommand::GetDirectionString() const
{
    if(_shootVector == Position(0,-1)) { return "N";}
    if(_shootVector == Position(1,-1)) { return "NE";}
    if(_shootVector == Position(1,0)) { return "E";}
    if(_shootVector == Position(1,1)) { return "SE";}
    if(_shootVector == Position(0,1)) { return "S";}
    if(_shootVector == Position(-1,1)) { return "SW";}
    if(_shootVector == Position(-1,0)) { return "W";}
    if(_shootVector == Position(-1,-1)) { return "NW";}
    return "";
}

std::string ShootCommand::GetCommandString() const
{
    std::stringstream ret;
    ret << "shoot " << GetDirectionString();
    return ret.str();
}