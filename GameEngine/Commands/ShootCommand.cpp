#include "ShootCommand.hpp"
#include <iostream>
#include <algorithm>

ShootCommand::ShootCommand(bool player1, std::shared_ptr<GameState> state, ShootCommand::ShootDirection dir) :
    Command(player1, state)
{
    _order = 3;
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

void ShootCommand::Execute() const
{
    Position pos = _worm->position + _shootVector;

    while (pos.x < GameConfig::mapSize &&
            pos.y < GameConfig::mapSize &&
            _worm->position.MovementDistanceTo(pos) <= _worm->weapon.range) {

        //check diag
        if( (pos.x != _worm->position.x && pos.y != _worm->position.y) && //i.e. we are shooting diagonally
            _worm->position.MovementDistanceTo(pos) > _worm->weapon.diagRange) {
            //std::cerr << "OUT OF RANGE! (" <<_worm->position.MovementDistanceTo(pos) << " > " << _worm->weapon.diagRange << ")" << std::endl;
            _player->command_score += GameConfig::scores.missedAttack;
            return;
        }

        Cell* cell = _state->Cell_at(pos);

        if (cell->type != CellType::AIR) {
            //std::cerr << "HIT DIRT!" << std::endl;
            _player->command_score += GameConfig::scores.missedAttack;
            return;
        }

        if (cell->worm != nullptr) {
            //std::cerr << "HIT A WORM!" << std::endl;
            auto wormRef = cell->worm;
            wormRef->TakeDamage(_worm->weapon.damage);

            if(wormRef->IsDead()) {
                _player->command_score += GameConfig::scores.killShot;
            } else {
                if(std::any_of(_player->worms.begin(), _player->worms.end(), [&](Worm& w){return &w == wormRef;})) {
                    _player->command_score += GameConfig::scores.friendlyFire;
                } else {
                    _player->command_score += GameConfig::scores.attack;
                }
            }

            return;
        }

        pos += _shootVector;
    }

    //if we get this far, shot didn't hit anything
    _player->command_score += GameConfig::scores.missedAttack;
}

bool ShootCommand::IsValid() const
{
    return true; //always valid!
}