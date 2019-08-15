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
    static void Initialise();

    GameState(const GameState& p);

    void SetCellTypeAt(Position pos, CellType type);
    static void AddLavaAt(Position pos, int roundNum = -1);

    void PlacePowerupAt(Position pos);
    void ClearPowerupAt(Position pos);
    void PlaceWormAt(Position pos, Worm* worm);
    static std::shared_ptr<Command> Str2Cmd(std::string str);

    void Move_worm(Worm* worm, Position pos);

    Player* GetPlayer(bool player1);

    std::vector<Worm*> WormsWithinDistance(Position pos, int dist);
    Position Closest_dirt(const Position& fromPos);
    int Dist_to_closest_enemy(bool player1);

    void MarkDirtDugThisRound(Position pos);
    bool DirtWasDugThisRound(Position pos);
    void ClearDirtsDugThisRound();

    void MarkLavaRemovedThisRound(Position pos);
    bool LavaWasRemovedThisRound(Position pos);
    void ClearLavasRemovedThisRound();

    bool operator==(const GameState &other) const;

    private:

    //for bitsets [0] is the LSB.  So it is stored x/y flipped with respect to map coords, to make calcs easier.    
    std::bitset<MAP_SIZE*MAP_SIZE> mapDeepSpaces; //TODO no need to store this actually, it is always the same
    std::bitset<MAP_SIZE*MAP_SIZE> mapDirts;

    static bool haveSetup;
    static std::bitset<MAP_SIZE*MAP_SIZE> mapLavas[400]; //one for each round :)
    static std::bitset<MAP_SIZE*MAP_SIZE> bananaBombOverlay;
    static int bananaBombOverlayCentre;

    std::vector<Position> healthPackPos;
    std::vector<Position> dirtsDugThisRound;
    std::vector<Position> lavasRemovedThisRound;

    static void SetupBananaBombOverlay();
    static void SetupLavas();
    void UpdateRefs();
    void UpdateRefs(Player& player);

    public:

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

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
        if(LavaWasRemovedThisRound(pos)) {
            return false;
        }

        auto posBit = (MAP_SIZE*pos.y + pos.x);
        return mapLavas[roundNumber-1][posBit];
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
        //not using ForAllWorms for performance reasons
        for(auto & w : player1.worms) { 
            if(w.position == pos && (!w.IsDead() || w.diedByLavaThisRound) ) {
                return &w;
            }
         }
        for(auto & w : player2.worms) { 
            if(w.position == pos && (!w.IsDead() || w.diedByLavaThisRound) ) {
                return &w;
            }
         }

        return nullptr;
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

    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    inline void ForAllWorms(std::function<void(Worm&)> wormFn)
    {
        for(auto & worm : player1.worms) { wormFn(worm); }
        for(auto & worm : player2.worms) { wormFn(worm); }
    }

    inline void ForAllLiveWorms(std::function<void(Worm&)> wormFn)
    {
        for(auto & worm : player1.worms) {
            if(!worm.IsDead()) {
                wormFn(worm);
            }
        }
        for(auto & worm : player2.worms) {
            if(!worm.IsDead()) {
                wormFn(worm);
            }
        }
    }

    inline void ForAllLiveWorms(bool _player1, std::function<void(Worm&)> wormFn)
    {
        if(_player1) {
            for(auto & worm : player1.worms) {
                if(!worm.IsDead()) {
                    wormFn(worm);
                }
            }
        } else {
            for(auto & worm : player2.worms) {
                if(!worm.IsDead()) {
                    wormFn(worm);
                }
            }
        }
    }
};

#endif
