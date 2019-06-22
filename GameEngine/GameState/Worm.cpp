#include "Worm.hpp"
#include "GameState.hpp"

Worm::Worm(GameState* _state) : state{_state}, id{0}, movedThisRound{0}
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

bool Worm::operator==(const Worm &other) const
{
    //std::cerr << "(" << __FUNCTION__ << ") " 
    //" id: " << (id == other.id) << "(" << id << ", " << other.id << ")" <<
    //" health: " << (health == other.health) << "( " << health << ", " << other.health << ")" <<
    //" position: " << (position == other.position) <<
    //" previous_position: " << (previous_position == other.previous_position) <<
    //" weapon: " << (weapon == other.weapon) <<
    //" diggingRange: " << (diggingRange == other.diggingRange) <<
    //" movementRange: " << (movementRange == other.movementRange) <<
    //std::endl;

    return (id == other.id &&
            health == other.health &&
            position == other.position &&
            //previous_position == other.previous_position && //not loaded from state (this operator only used for unit testing)
            weapon == other.weapon &&
            diggingRange == other.diggingRange &&
            movementRange == other.movementRange);
}
