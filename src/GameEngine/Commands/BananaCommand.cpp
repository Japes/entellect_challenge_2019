#include "BananaCommand.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

std::vector<std::vector<int>> BananaCommand::_splashDamageLookup;

BananaCommand::BananaCommand(Position pos) : _pos{pos}
{
    _order = 3;

    if(_splashDamageLookup.empty()) {
        std::vector<int> row1 {0,0,7,0,0};
        std::vector<int> row2 {0,11,13,11,0};
        std::vector<int> row3 {7,13,20,13,7};
        _splashDamageLookup.push_back(row1);
        _splashDamageLookup.push_back(row2);
        _splashDamageLookup.push_back(row3);
        _splashDamageLookup.push_back(row2);
        _splashDamageLookup.push_back(row1);
    }
}

void BananaCommand::Execute(bool player1, std::shared_ptr<GameState> state) const
{
    /*
        splash damage:

        .   .   .   .   .   .   .
        .   .   .   7   .   .   .
        .   .   11  13  11  .   .
        .   7   13  20  13  7   .
        .   .   11  13  11  .   .
        .   .   .   7   .   .   .
        .   .   .   .   .   .   .
    */

    Player* player = state->GetPlayer(player1);
    Worm* worm = player->GetCurrentWorm();

    player->consecutiveDoNothingCount = 0;
    --worm->banana_bomb_count;

    if(state->CellType_at(_pos) == CellType::DEEP_SPACE) {
        return; //wasted your bomb...
    }

    //do the explosion:
    int points = 0;
    for(unsigned x = 0; x < _splashDamageLookup.size(); ++x) {
        for(unsigned y = 0; y < _splashDamageLookup.size(); ++y) {

            Position mapPos = _pos + Position(x-2, y-2);
            if(!mapPos.IsOnMap()) {
                continue;
            }
            auto dmg = _splashDamageLookup[x][y];
            if(dmg == 0) {
                continue;
            }

            //clear dirt
            if(state->CellType_at(mapPos) == CellType::DIRT) {
                state->SetCellTypeAt(mapPos, CellType::AIR);
                state->RemoveLavaAt(mapPos);
                points += GameConfig::scores.dig;
            } 

            //destroy powerup
            if(state->PowerUp_at(mapPos) != nullptr) {
                state->ClearPowerupAt(mapPos);
            }
            
            //hurt worms
            auto hitWorm = state->Worm_at(mapPos);
            if(hitWorm != nullptr) {
                int dmgPoints = 0;
                hitWorm->TakeDamage(dmg);
                dmgPoints += dmg*2;

                if(hitWorm->IsDead()) {
                    dmgPoints += GameConfig::scores.killShot;
                }
                if(std::any_of(player->worms.begin(), player->worms.end(), [&](Worm& w){return &w == hitWorm;})) {
                    dmgPoints *= -1;
                }
                points += dmgPoints;
            }
        }
    }

    player->command_score += points;
}


bool BananaCommand::IsValid(bool player1, std::shared_ptr<GameState> state) const
{
    Player* player = state->GetPlayer(player1);
    Worm* worm = player->GetCurrentWorm();

    if(!_pos.IsOnMap()) {
        std::cerr << latestBot << " (" << __FUNCTION__ << ") banana target not on map: " << _pos << std::endl;
        return false;
    }

    if(worm->proffession != Worm::Proffession::AGENT) {
        std::cerr << latestBot << " (" << __FUNCTION__ << ") only agents can throw bananas!" << std::endl;
        return false;
    }

    if(worm->banana_bomb_count <= 0) {
        std::cerr << latestBot << " (" << __FUNCTION__ << ") This guy doesn't have any bananas left!" << std::endl;
        return false;
    }

    if(!worm->position.BananaSnowballCanReach(_pos)) {
        std::cerr << latestBot << " (" << __FUNCTION__ << ") " << _pos << " is too far to throw a banana from " << worm->position << "!" << std::endl;
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
