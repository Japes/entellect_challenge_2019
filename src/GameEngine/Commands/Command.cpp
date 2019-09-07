#include "Command.hpp"

const std::string Command::latestBot = "JP36_beatjp37";

//to help with debugging...
std::ostream & operator << (std::ostream &out, std::shared_ptr<Command> &cmd)
{
    out << "(" << cmd->GetCommandString() << ")";
    return out;
}
