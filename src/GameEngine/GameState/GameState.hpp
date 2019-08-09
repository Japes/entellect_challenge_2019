#ifndef GAME_STATE_H
#define GAME_STATE_H

#include "GameConfig.hpp"
#include "Player.hpp"
#include "Cell.hpp"
#include "./rapidjson/document.h"
#include <functional>
#include <bitset>

#define first_bit_set (0x8000000000000000)

//current state of the game
class GameState
{
	public:
    Player player1;
    Player player2;
    int roundNumber;
    PowerUp healthPack; //just here so that cells can reference something static

    GameState();
    GameState(const GameState& p);
    GameState(rapidjson::Document& roundJSON);

    void SetCellTypeAt(Position pos, CellType type);
    void RemoveLavaAt(Position pos);
    void AddLavaAt(Position pos);
    void PlacePowerupAt(Position pos);
    void ClearPowerupAt(Position pos);
    void PlaceWormAt(Position pos, Worm* worm);
    static std::shared_ptr<Command> Str2Cmd(std::string str);

    void Move_worm(Worm* worm, Position pos);

    Player* GetPlayer(bool player1);
    void ForAllWorms(std::function<void(Worm&)> wormFn);

    std::vector<Worm*> WormsWithinDistance(Position pos, int dist);
    Position Closest_dirt(const Position& fromPos);
    int Dist_to_closest_enemy(bool player1);

    bool operator==(const GameState &other) const;

    private:

    //for bitsets [0] is the LSB.  So it is stored x/y flipped with respect to map coords, to make calcs easier.    
    std::bitset<MAP_SIZE*MAP_SIZE> mapDeepSpaces; //TODO no need to store this actually, it is always the same
    std::bitset<MAP_SIZE*MAP_SIZE> mapDirts;
    std::bitset<MAP_SIZE*MAP_SIZE> mapLavas;
    static std::bitset<MAP_SIZE*MAP_SIZE> bananaBombOverlay;
    static int bananaBombOverlayCentre;
    std::vector<Position> healthPackPos;

    void UpdateRefs();
    void UpdateRefs(Player& player);

    public:

    inline CellType CellType_at(Position pos)
    {
        auto posBit = (MAP_SIZE*pos.y + pos.x);
        if(mapDeepSpaces[posBit]) {
            return CellType::DEEP_SPACE;
        } 

        if(mapDirts[posBit]) {
            return CellType::DIRT;
        } 
            
        return CellType::AIR;
    }

    inline bool LavaAt(Position pos)
    {
        auto posBit = (MAP_SIZE*pos.y + pos.x);
        return mapLavas[posBit];
    }

    inline PowerUp* PowerUp_at(Position pos)
    {
        for(auto & hpPos : healthPackPos) {
            if(hpPos == pos) {
                return &healthPack;
            } 
        }

        return nullptr;
    }

    inline Worm* Worm_at(Position pos)
    {
        Worm* ret = nullptr;

        //not using ForAllWorms here for performance reasons
        for(auto & w : player1.worms) {
            if(w.position == pos && !w.IsDead()) {
                ret = &w;
            }
        }
        for(auto & w : player2.worms) {
            if(w.position == pos && !w.IsDead()) {
                ret = &w;
            }
        }

        return ret;
    }

    inline int DirtsBananaWillHit(const Position& pos) {
        
        int ret = 0;
        
        if(!pos.IsOnMap() || CellType_at(pos) == CellType::DEEP_SPACE ) {
            return ret;
        }

        auto posBit = (MAP_SIZE*pos.y + pos.x);

        if(posBit > bananaBombOverlayCentre) {
            return (mapDirts & (bananaBombOverlay << (posBit - bananaBombOverlayCentre)) ).count();
        }
        return (mapDirts & (bananaBombOverlay >> (bananaBombOverlayCentre - posBit)) ).count();
    }
};

#endif
