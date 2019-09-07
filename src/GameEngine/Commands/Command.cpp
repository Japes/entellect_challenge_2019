#include "Command.hpp"

const std::string Command::latestBot = "jp36_btjp37_asm";

//to help with debugging...
std::ostream & operator << (std::ostream &out, std::shared_ptr<Command> &cmd)
{
    out << "(" << cmd->GetCommandString() << ")";
    return out;
}
