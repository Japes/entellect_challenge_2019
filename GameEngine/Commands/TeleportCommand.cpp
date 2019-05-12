#include "TeleportCommand.hpp"
#include <iostream>

TeleportCommand::TeleportCommand(bool player1, std::shared_ptr<GameState> state, Position pos) :
    Command(player1, state),
    _pos{pos}
{
    _order = 2;
}

void TeleportCommand::Execute() const
{
    /*
        val targetCell = gameMap[target]
        val occupier = targetCell.occupier
        if (occupier != null && wormsCollide(gameMap, worm, occupier)) {
            worm.takeDamage(config.pushbackDamage, gameMap.currentRound)
            occupier.takeDamage(config.pushbackDamage, gameMap.currentRound)

            // 50% chance to pushback or swap positions
            if (random.nextBoolean()) {
                pushbackWorms(worm, occupier, gameMap)
            } else {
                swapWorms(worm, occupier, gameMap)
            }
        } else {
            worm.moveTo(gameMap, target)
        }

        return StandardCommandFeedback(this.toString(), playerId = worm.player.id, score = config.scores.move)
        */
}

bool TeleportCommand::IsValid() const
{
    if (_pos.x >= MAP_SIZE || _pos.y >= MAP_SIZE ||
        _pos.x < 0 || _pos.y < 0 ) {
        return false;
    }

    if(_state->map[_pos.x][_pos.y].type != CellType::AIR) {
        return false;
    }

    if (_worm->position.MovementDistanceTo(_pos) > _worm->movementRange) {
        return false;
    }

    return true;

/*
        val occupier = targetCell.occupier
        if (occupier != null && !wormsCollide(gameMap, worm, occupier)) {
            return CommandValidation.invalidMove("Target occupied")
        }

        return CommandValidation.validMove()
*/
}

/*
    private fun wormsCollide(gameMap: WormsMap, movingWorm: Worm, occupier: Worm): Boolean {
        return occupier != movingWorm && occupier.roundMoved == gameMap.currentRound
    }

        private fun pushbackWorms(worm: Worm, occupier: Worm, gameMap: WormsMap) {
        val wormPosition = worm.position
        val occupierPosition = occupier.previousPosition

        worm.moveTo(gameMap, wormPosition)
        occupier.moveTo(gameMap, occupierPosition)
    }

    private fun swapWorms(worm: Worm, occupier: Worm, gameMap: WormsMap) {
        val wormPosition = worm.position
        val occupierPosition = occupier.previousPosition

        worm.moveTo(gameMap, occupierPosition)
        occupier.moveTo(gameMap, wormPosition)
    }
*/