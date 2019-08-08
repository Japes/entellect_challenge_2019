#include "GameState.hpp"
#include "AllCommands.hpp"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include <cmath>

std::bitset<MAP_SIZE*MAP_SIZE> GameState::bananaBombOverlay;
int GameState::bananaBombOverlayCentre;

GameState::GameState() :
    player1(this),
    player2(this),
    roundNumber{1}
{
    player1.id = 1;
    player2.id = 2;

    if(bananaBombOverlay.none()) {
        //initialise to the shadow of a bomb in the topish leftish corner
        bananaBombOverlay.set(2);

        bananaBombOverlay.set(MAP_SIZE + 1);
        bananaBombOverlay.set(MAP_SIZE + 2);
        bananaBombOverlay.set(MAP_SIZE + 3);

        bananaBombOverlay.set(MAP_SIZE*2);
        bananaBombOverlay.set(MAP_SIZE*2 + 1);
        bananaBombOverlay.set(MAP_SIZE*2 + 2);
        bananaBombOverlay.set(MAP_SIZE*2 + 3);
        bananaBombOverlay.set(MAP_SIZE*2 + 4);

        bananaBombOverlay.set(MAP_SIZE*3 + 1);
        bananaBombOverlay.set(MAP_SIZE*3 + 2);
        bananaBombOverlay.set(MAP_SIZE*3 + 3);

        bananaBombOverlay.set(MAP_SIZE*4 + 2);

        bananaBombOverlayCentre = MAP_SIZE*2 + 2;

        //std::cerr << "(" << __FUNCTION__ << ") bananaBombOverlay: " << bananaBombOverlay << std::endl;
        //std::cerr << "(" << __FUNCTION__ << ") bananaBombOverlay << 1: " << (bananaBombOverlay << 1) << std::endl;
        //std::cerr << "(" << __FUNCTION__ << ") bananaBombOverlay << -1: " << (bananaBombOverlay << -1) << std::endl;
    }
}

//deep copy of state
GameState::GameState(const GameState& other) :
    player1{other.player1},
    player2{other.player2},
    roundNumber{other.roundNumber},
    healthPack{other.healthPack},
    mapDeepSpaces{other.mapDeepSpaces},
    mapDirts{other.mapDirts},
    mapLavas{other.mapLavas},
    healthPackPos{other.healthPackPos}
{
    UpdateRefs();
}

void GameState::UpdateRefs()
{
    player1.state = this;
    UpdateRefs(player1);

    player2.state = this;
    UpdateRefs(player2);
}

void GameState::UpdateRefs(Player& player)
{
    for(Worm &w : player.worms) {
        w.state = this;
        if(w.health > 0) {
            PlaceWormAt(w.position, &w); //so that cell's worm is updated
        }
    }
}

void GameState::SetCellTypeAt(Position pos, CellType type)
{
    auto posBit = (MAP_SIZE*pos.y + pos.x);

    switch(type) {
        case CellType::AIR:
            mapDeepSpaces.reset(posBit);
            mapDirts.reset(posBit);
        break;
        case CellType::DEEP_SPACE:
            mapDirts.reset(posBit);
            mapDeepSpaces.set(posBit);
        break;
        case CellType::DIRT:
            mapDeepSpaces.reset(posBit);
            mapDirts.set(posBit);
        break;
    }
}

void GameState::AddLavaAt(Position pos)
{
    auto posBit = (MAP_SIZE*pos.y + pos.x);
    mapLavas.set(posBit);
}

void GameState::PlacePowerupAt(Position pos)
{
    healthPackPos.push_back(pos);
}

void GameState::ClearPowerupAt(Position pos)
{
    auto it = healthPackPos.begin();

    while(it != healthPackPos.end()) {
        if(*it == pos) {
            healthPackPos.erase(it);
            return;
        }
        ++it;
    }
}

void GameState::PlaceWormAt(Position pos, Worm* worm)
{
    worm->position = pos;
}

void GameState::Move_worm(Worm* worm, Position pos)
{
    worm->previous_position = worm->position;

    PlaceWormAt(pos, worm);
}

Player* GameState::GetPlayer(bool player1)
{
    return player1 ? &this->player1 : &player2;
}

void GameState::ForAllWorms(std::function<void(Worm&)> wormFn)
{
    for(auto & worm : player1.worms) {
        wormFn(worm);
    }
    for(auto & worm : player2.worms) {
        wormFn(worm);
    }
}

//returns closest dirt in terms of move distance
//returns {-1, -1} if no dirts
Position GameState::Closest_dirt(const Position& fromPos) 
{
    //spiral around from worm and return the first dirt u find
    unsigned extents = 1;
    while(extents < (MAP_SIZE)) {

        Position NW = fromPos + Position(-extents,-extents);
        NW.ClampToMap();
        Position NE = fromPos + Position( extents,-extents);
        NE.ClampToMap();
        Position SE = fromPos + Position( extents, extents);
        SE.ClampToMap();
        Position SW = fromPos + Position(-extents, extents);
        SW.ClampToMap();

        for(Position pos = NW; pos != NE; ++pos.x) {
             if(CellType_at(pos) == CellType::DIRT) {
                return pos;
            }   
        }
        for(Position pos = NE; pos != SE; ++pos.y) {
             if(CellType_at(pos) == CellType::DIRT) {
                return pos;
            }   
        }
        for(Position pos = SE; pos != SW; --pos.x) {
             if(CellType_at(pos) == CellType::DIRT) {
                return pos;
            }   
        }
        for(Position pos = SW; pos != NW; --pos.y) {
             if(CellType_at(pos) == CellType::DIRT) {
                return pos;
            }   
        }

        ++extents;
    }

    return {-1,-1};
}

int GameState::Dist_to_closest_enemy(bool player1) 
{
    //get distance to closest enemy
    //pos of my worm:
    Player * me = GetPlayer(player1);
    Worm * worm = me->GetCurrentWorm();
    Position myWormPos = worm->position;

    //now calc
    Player * enemy = GetPlayer(!player1);
    int closestDist = 9999;
    for(auto const & worm : enemy->worms) {
        if(worm.IsDead() || !worm.position.IsOnMap()) {
            continue;
        }
        auto dist = myWormPos.EuclideanDistanceTo(worm.position);
        if(dist < closestDist){
            closestDist = dist;
        }
    }

    return closestDist;
}

//return all worms if dist is -1
std::vector<Worm*> GameState::WormsWithinDistance(Position pos, int dist)
{
    std::vector<Worm*> ret;

    ForAllWorms([&](Worm& worm) {
        if(dist < 0 || (!worm.IsDead() && pos.EuclideanDistanceTo(worm.position) <= dist)) {
            ret.push_back(&worm);
        }
    });

    return ret;
}

bool GameState::operator==(const GameState &other) const
{
    //std::cerr << "(" << __FUNCTION__ << ") deepSpacesGood: " << deepSpacesGood <<
    //" dirtsGood: " << dirtsGood << 
    //" healthPackPos == other.healthPackPos: " << (healthPackPos == other.healthPackPos) <<
    //" roundNumber == other.roundNumber: " << (roundNumber == other.roundNumber) <<
    //std::endl;

    return mapDeepSpaces == other.mapDeepSpaces && 
            mapDirts == other.mapDirts && 
            mapLavas == other.mapLavas && 
            player1 == other.player1 && 
            player2 == other.player2 &&
            healthPackPos == other.healthPackPos &&
            roundNumber == other.roundNumber;
}
