#include "Worm.hpp"
#include "GameState.hpp"

Worm::Worm(GameState* _state) : state{_state}, id{0}
{
    health = GameConfig::commandoWorms.initialHp;
    diggingRange = GameConfig::commandoWorms.diggingRange;
    movementRange = GameConfig::commandoWorms.movementRange;
}

bool Worm::IsDead()
{
    return (health <= 0);
}

void Worm::TakeDamage(int dmgAmount)
{
    health -= dmgAmount;
    if(IsDead()) {
        state->Cell_at(position)->worm = nullptr;
    }
    state->player1.RecalculateHealth();
    state->player2.RecalculateHealth();
}
