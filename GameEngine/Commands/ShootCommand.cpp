#include "ShootCommand.hpp"
#include <iostream>

ShootCommand::ShootCommand(bool player1, std::shared_ptr<GameState> state, ShootCommand::ShootDirection dir) :
    Command(player1, state)
{
    _order = 2;
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
    std::cout << "ShootCommand::Execute()" << std::endl;
    Position pos = _worm->position + _shootVector;
    std::cout << "pos: " << pos << std::endl;

    while (pos.x < GameConfig::mapSize &&
            pos.y < GameConfig::mapSize &&
            _worm->position.MovementDistanceTo(pos) <= _worm->weapon.range) {

        std::cout << "  -pos: " << pos << std::endl;

        //check diag
        if( (pos.x != _worm->position.x && pos.y != _worm->position.y) && //i.e. we are shooting diagonally
            _worm->position.MovementDistanceTo(pos) > _worm->weapon.diagRange) {
            std::cout << "OUT OF RANGE! (" <<_worm->position.MovementDistanceTo(pos) << " > " << _worm->weapon.diagRange << ")" << std::endl;
            return;
        }

        Cell* cell = _state->Cell_at(pos);

        if (cell->type != CellType::AIR) {
            std::cout << "HIT DIRT!" << std::endl;
            return;
        }

        if (cell->worm != nullptr) {
            std::cout << "HIT A WORM!" << std::endl;
            cell->worm->health -= _worm->weapon.damage;
            _state->player1.RecalculateHealth();
            _state->player2.RecalculateHealth();
            return;
        }

        pos += _shootVector;
    }
}

bool ShootCommand::IsValid() const
{
    return true; //always valid!
}