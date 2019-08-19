#ifndef SHOOT_COMMAND_H
#define SHOOT_COMMAND_H

#include "Command.hpp"

class ShootCommand : public Command
{
	public:
    enum class ShootDirection : int {
        N,  //UP        (0, -1),
        NE, //UP_RIGHT  (1, -1),
        E,  //RIGHT     (1, 0),
        SE, //DOWN_RIGHT(1, 1),
        S,  //DOWN      (0, 1),
        SW, //DOWN_LEFT (-1, 1),
        W,  //LEFT      (-1, 0),
        NW  //UP_LEFT   (-1, -1);
    };

    static const Position noShot;

    ShootCommand(ShootCommand::ShootDirection dir);
    ShootCommand(Position dir);
    void Execute(bool player1, GameStatePtr state) const override;
    bool IsValid(bool player1, GameStatePtr state) const override;
    std::string GetCommandString() const override;
    
    static Worm* WormOnTarget(const Worm* worm, const GameStatePtr state, const Position& shootvector);
    static Position GetValidShot(const Worm& shootingWorm, const Position& target, GameStatePtr state);

    static bool ClearShot(const Worm* worm, const GameStatePtr state, const Position& shootvector, const Position& targetPos);

    bool operator==(const ShootCommand& other);

    private:
    std::string GetDirectionString() const;
    Position _shootVector; //e.g. south east is {-1, 1}
};

#endif
