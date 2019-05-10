#include "DigCommand.hpp"

DigCommand::DigCommand(Position pos) :
    _pos{pos}
{
    _order = 1;
}

void DigCommand::Execute(bool player1, GameState& state) const
{
        val targetCell = gameMap[target]
        targetCell.type = CellType.AIR

        return StandardCommandFeedback(this.toString(), score = config.scores.dig, playerId = worm.player.id)
}

bool DigCommand::IsValid() const
{
            if (target !in gameMap) {
            return CommandValidation.invalidMove("$target out of map bounds")
        }

        val targetCell = gameMap[target]

        if (!targetCell.type.diggable) {
            return CommandValidation.invalidMove("Cell type ${targetCell.type} not diggable")
        }

        if (target.movementDistance(worm.position) > worm.diggingRange) {
            return CommandValidation.invalidMove("Cell $target too far away")
        }

        return CommandValidation.validMove()

    return true;
}
