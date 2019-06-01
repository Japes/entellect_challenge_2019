#include "ShootCommand.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

ShootCommand::ShootCommand(ShootCommand::ShootDirection dir)
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

void ShootCommand::Execute(bool player1, std::shared_ptr<GameState> state) const
{
    Player* player = player1 ? &state->player1 : &state->player2;
    Worm* worm = &player->worms[player->currentWormId-1];

    player->consecutiveDoNothingCount = 0;

    Position pos = worm->position + _shootVector;

    while (pos.IsOnMap() && worm->position.MovementDistanceTo(pos) <= worm->weapon.range) {

        //check diag
        if( (pos.x != worm->position.x && pos.y != worm->position.y) && //i.e. we are shooting diagonally
            worm->position.MovementDistanceTo(pos) > worm->weapon.diagRange) {
            //std::cerr << "OUT OF RANGE! (" <<worm->position.MovementDistanceTo(pos) << " > " << worm->weapon.diagRange << ")" << std::endl;
            player->command_score += GameConfig::scores.missedAttack;
            return;
        }

        Cell* cell = state->Cell_at(pos);

        if (cell->type != CellType::AIR) {
            //std::cerr << "HIT DIRT!" << std::endl;
            player->command_score += GameConfig::scores.missedAttack;
            return;
        }

        if (cell->worm != nullptr) {
            //std::cerr << "HIT A WORM!" << std::endl;
            auto wormRef = cell->worm;
            wormRef->TakeDamage(worm->weapon.damage);

            if(wormRef->IsDead()) {
                player->command_score += GameConfig::scores.killShot;
            } else {
                if(std::any_of(player->worms.begin(), player->worms.end(), [&](Worm& w){return &w == wormRef;})) {
                    player->command_score += GameConfig::scores.friendlyFire;
                } else {
                    player->command_score += GameConfig::scores.attack;
                }
            }

            return;
        }

        pos += _shootVector;
    }

    //if we get this far, shot didn't hit anything
    player->command_score += GameConfig::scores.missedAttack;
}

bool ShootCommand::IsValid(bool player1, std::shared_ptr<GameState> state) const
{
    return true; //always valid!
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