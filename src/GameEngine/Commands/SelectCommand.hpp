#ifndef SELECT_COMMAND_H
#define SELECT_COMMAND_H

#include "Command.hpp"

class SelectCommand : public Command
{
	public:

    SelectCommand(int wormIndex, std::shared_ptr<Command> selectedCmd);
    void Execute(bool player1, std::shared_ptr<GameState> state) const override;
    bool IsValid(bool player1, std::shared_ptr<GameState> state) const override;
    std::string GetCommandString() const override;

    bool operator==(const SelectCommand& other);

    private:
    int _wormIndex;
    std::shared_ptr<Command> _selectedCmd;
};

#endif
