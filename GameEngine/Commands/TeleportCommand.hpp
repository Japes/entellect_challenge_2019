#ifndef TELEPORT_COMMAND_H
#define TELEPORT_COMMAND_H

#include "Command.hpp"

class TeleportCommand : public Command
{
	public:
    Position _pos;

    TeleportCommand(bool player1, std::shared_ptr<GameState> state, Position pos, bool* forceRandom = nullptr);
    void Execute() const override;
    bool IsValid() const override;

    bool WormMovedThisRound(const Worm* worm) const;

    bool FiftyFiftyChance() const;

    bool operator==(const TeleportCommand& other);

    private:
    bool* _forceRandom;
    static bool _randomReturnFlipFlop;
};

#endif
