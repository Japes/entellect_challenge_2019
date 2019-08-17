#ifndef TELEPORT_COMMAND_H
#define TELEPORT_COMMAND_H

#include "Command.hpp"

class TeleportCommand : public Command
{
	public:
    Position _pos;

    TeleportCommand(Position pos, bool* forceRandom = nullptr);
    void Execute(bool player1, GameStatePtr state) const override;
    bool IsValid(bool player1, GameStatePtr state) const override;
    std::string GetCommandString() const override;

    bool FiftyFiftyChance() const;

    bool operator==(const TeleportCommand& other);

    private:
    bool* _forceRandom;
    static bool _randomReturnFlipFlop;
};

#endif
