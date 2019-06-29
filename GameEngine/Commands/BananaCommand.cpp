#include "BananaCommand.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

BananaCommand::BananaCommand(Position pos) : _pos{pos}
{
}

void BananaCommand::Execute(bool player1, std::shared_ptr<GameState> state) const
{
    Player* player = player1 ? &state->player1 : &state->player2;
    Worm* worm = &player->worms[player->currentWormId-1];

    player->consecutiveDoNothingCount = 0;

/*
    Worm* hitworm = WormOnTarget(player1, state, _shootVector);

    if(hitworm == nullptr) {
        player->command_score += GameConfig::scores.missedAttack;
        return;
    }

    hitworm->TakeDamage(worm->weapon.damage);
*/
    int points = worm->weapon.damage*2;

  /*  
    if(hitworm->IsDead()) {
        points += GameConfig::scores.killShot;
    }

    if(std::any_of(player->worms.begin(), player->worms.end(), [&](Worm& w){return &w == hitworm;})) {
        points *= -1;
    }
    */

    --worm->banana_bomb_count;

    player->command_score += points;
}


bool BananaCommand::IsValid(bool player1, std::shared_ptr<GameState> state) const
{
    Player* player = player1 ? &state->player1 : &state->player2;
    Worm* worm = player->GetCurrentWorm();

    if(worm->proffession != Worm::Proffession::AGENT) {
        std::cerr << "(" << __FUNCTION__ << ") only agents can throw bananas!" << std::endl;
        return false;
    }

    if(worm->banana_bomb_count <= 0) {
        std::cerr << "(" << __FUNCTION__ << ") This guy doesn't have any bananas left!" << std::endl;
        return false;
    }

    auto shootDist = worm->position.ShootDistanceTo(_pos);
    if(shootDist > GameConfig::agentWorms.banana.range) {
        std::cerr << "(" << __FUNCTION__ << ") " << shootDist << " is too far to throw a banana!" << std::endl;
        return false;
    }

    return true; //always valid!
}

bool BananaCommand::operator==(const BananaCommand& other)
{
    return _pos == other._pos;
}

std::string BananaCommand::GetCommandString() const
{
    std::stringstream ret;
    ret << "banana " << _pos.x << " " << _pos.y;
    return ret.str();
}
