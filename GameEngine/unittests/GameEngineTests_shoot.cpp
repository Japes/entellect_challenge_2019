#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"

void place_worm(bool player1, int wormNumber, Position pos, std::shared_ptr<GameState> state)
{
    Worm* worm_under_test;
    if(player1) {
        worm_under_test = &state->player1.worms[wormNumber - 1];
    } else {
        worm_under_test = &state->player2.worms[wormNumber - 1];
    }
    worm_under_test->position = worm_under_test->previous_position = pos;
    state->Cell_at(pos)->worm = worm_under_test;
}

bool shot_hits(std::shared_ptr<GameState> state, int targetWormNumber, ShootCommand::ShootDirection dir, bool friendlyWorm = false)
{
    Player* target_player = friendlyWorm? &state->player1 : &state->player2;
    Worm* target_worm = &target_player->worms[targetWormNumber - 1];

    int targetPlayerStartingHealth = target_player->health;
    int targetWormStartingHealth = target_worm->health;
    int player1StartingDoNothing = state->player1.consecutiveDoNothingCount;

    ShootCommand player1move(true, state, dir);
    DoNothingCommand player2move(false, state);

    GameEngine eng(state);
    eng.AdvanceState(player1move,player2move);

    bool hit = (state->player1.consecutiveDoNothingCount == player1StartingDoNothing);
    hit &= (target_worm->health == targetWormStartingHealth - GameConfig::commandoWorms.weapon.damage);
    hit &= (target_player->health == targetPlayerStartingHealth - GameConfig::commandoWorms.weapon.damage);

    //std::cout << "state->player1.consecutiveDoNothingCount: " << state->player1.consecutiveDoNothingCount << 
    //" target_worm->health: " << targetWormStartingHealth <<
    //" target_player->health: " << targetPlayerStartingHealth <<
    //std::endl;

    return hit;
}

void check_shot_hit(std::shared_ptr<GameState> state, int targetWormNumber, ShootCommand::ShootDirection dir, bool friendlyWorm = false)
{
    THEN("Shot hits")
    {
        bool hit = shot_hits(state, targetWormNumber, dir, friendlyWorm);
        REQUIRE(hit);
    }
}

void check_shot_missed(std::shared_ptr<GameState> state,ShootCommand::ShootDirection dir)
{
    int player2StartingHealth = state->player2.health;
    int player1StartingHealth = state->player2.health;
    int player1StartingDoNothing = state->player1.consecutiveDoNothingCount;

    ShootCommand player1move(true, state, dir);
    DoNothingCommand player2move(false, state);

    GameEngine eng(state);
    eng.AdvanceState(player1move,player2move);

    CHECK(state->player1.consecutiveDoNothingCount == player1StartingDoNothing);

    CHECK(state->player1.health == player1StartingHealth);
    CHECK(state->player2.health == player2StartingHealth);

    for(auto const& w : state->player1.worms) {
        CHECK(w.health == GameConfig::commandoWorms.initialHp);
    }
    for(auto const& w : state->player2.worms) {
        CHECK(w.health == GameConfig::commandoWorms.initialHp);
    }
}

//note shoot is always valid
TEST_CASE( "Shoot command hit close by", "[Shoot_command][hit_close_by]" ) {

    GIVEN("A game state and a shoot command")
    {
        auto state = std::make_shared<GameState>();

        Position worm_under_test_pos{10,10};
        place_worm(true, 1, worm_under_test_pos, state);

        THEN("Shooting hits worms close by N")
        {
            Position enemy_worm_pos1{9,9};
            place_worm(false, 1, enemy_worm_pos1, state);
            Position enemy_worm_pos2{10,9};
            place_worm(false, 2, enemy_worm_pos2, state);
            Position enemy_worm_pos3{11,9};
            place_worm(false, 3, enemy_worm_pos3, state);

            check_shot_hit(state, 1, ShootCommand::ShootDirection::NW);
            check_shot_hit(state, 2, ShootCommand::ShootDirection::N);
            check_shot_hit(state, 3, ShootCommand::ShootDirection::NE);
        }

        THEN("Shooting hits worms close by S")
        {
            Position enemy_worm_pos1{9,11};
            place_worm(false, 1, enemy_worm_pos1, state);
            Position enemy_worm_pos2{10,11};
            place_worm(false, 2, enemy_worm_pos2, state);
            Position enemy_worm_pos3{11,11};
            place_worm(false, 3, enemy_worm_pos3, state);

            check_shot_hit(state, 1, ShootCommand::ShootDirection::SW);
            check_shot_hit(state, 2, ShootCommand::ShootDirection::S);
            check_shot_hit(state, 3, ShootCommand::ShootDirection::SE);
        }

        THEN("Shooting hits worms close by EW")
        {
            Position enemy_worm_pos1{9,10};
            place_worm(false, 1, enemy_worm_pos1, state);
            Position enemy_worm_pos2{11,10};
            place_worm(false, 2, enemy_worm_pos2, state);

            check_shot_hit(state, 1, ShootCommand::ShootDirection::W);
            check_shot_hit(state, 2, ShootCommand::ShootDirection::E);
        }
    }

}

TEST_CASE( "Shoot command hit far away", "[Shoot_command]" ) {

    GIVEN("A game state and a shoot command")
    {
        auto state = std::make_shared<GameState>();

        Position worm_under_test_pos{15,15};
        place_worm(true, 1, worm_under_test_pos, state);

        auto placeRange = GameConfig::commandoWorms.weapon.range;
        auto placeDiagRange = GameConfig::commandoWorms.weapon.diagRange;

        THEN("Shooting hits worms far away N")
        {
            Position enemy_worm_pos1{worm_under_test_pos.x - placeDiagRange, worm_under_test_pos.y - placeDiagRange};
            place_worm(false, 1, enemy_worm_pos1, state);
            Position enemy_worm_pos2{worm_under_test_pos.x, worm_under_test_pos.y - placeRange};
            place_worm(false, 2, enemy_worm_pos2, state);
            Position enemy_worm_pos3{worm_under_test_pos.x + placeDiagRange, worm_under_test_pos.y - placeDiagRange};
            place_worm(false, 3, enemy_worm_pos3, state);

            check_shot_hit(state, 1, ShootCommand::ShootDirection::NW);
            check_shot_hit(state, 2, ShootCommand::ShootDirection::N);
            check_shot_hit(state, 3, ShootCommand::ShootDirection::NE);
        }

        THEN("Shooting hits worms far away S")
        {
            Position enemy_worm_pos1{worm_under_test_pos.x - placeDiagRange, worm_under_test_pos.y + placeDiagRange};
            place_worm(false, 1, enemy_worm_pos1, state);
            Position enemy_worm_pos2{worm_under_test_pos.x, worm_under_test_pos.y + placeRange};
            place_worm(false, 2, enemy_worm_pos2, state);
            Position enemy_worm_pos3{worm_under_test_pos.x + placeDiagRange, worm_under_test_pos.y + placeDiagRange};
            place_worm(false, 3, enemy_worm_pos3, state);

            check_shot_hit(state, 1, ShootCommand::ShootDirection::SW);
            check_shot_hit(state, 2, ShootCommand::ShootDirection::S);
            check_shot_hit(state, 3, ShootCommand::ShootDirection::SE);
        }

        THEN("Shooting hits worms far away EW")
        {
            Position enemy_worm_pos1{worm_under_test_pos.x - placeRange, worm_under_test_pos.y};
            place_worm(false, 1, enemy_worm_pos1, state);
            Position enemy_worm_pos2{worm_under_test_pos.x + placeRange, worm_under_test_pos.y};
            place_worm(false, 2, enemy_worm_pos2, state);

            check_shot_hit(state, 1, ShootCommand::ShootDirection::W);
            check_shot_hit(state, 2, ShootCommand::ShootDirection::E);
        }
    }
}

TEST_CASE( "Shoot command friendly fire", "[Shoot_command]" ) {

    GIVEN("A game state and a shoot command")
    {
        auto state = std::make_shared<GameState>();

        Position worm_under_test_pos{15,15};
        place_worm(true, 1, worm_under_test_pos, state);

        auto placeRange = GameConfig::commandoWorms.weapon.range;
        auto placeDiagRange = GameConfig::commandoWorms.weapon.diagRange;

        THEN("Shooting friendly worms 1")
        {
            Position enemy_worm_pos1{worm_under_test_pos.x - placeDiagRange, worm_under_test_pos.y - placeDiagRange};
            place_worm(true, 2, enemy_worm_pos1, state);
            Position enemy_worm_pos2{worm_under_test_pos.x, worm_under_test_pos.y - placeRange};
            place_worm(true, 3, enemy_worm_pos2, state);

            check_shot_hit(state, 2, ShootCommand::ShootDirection::NW, true);
            check_shot_hit(state, 3, ShootCommand::ShootDirection::N, true);
        }

        THEN("Shooting friendly worms 2")
        {
            Position enemy_worm_pos3{worm_under_test_pos.x + placeDiagRange, worm_under_test_pos.y - placeDiagRange};
            place_worm(true, 2, enemy_worm_pos3, state);
            Position enemy_worm_pos1{worm_under_test_pos.x - placeDiagRange, worm_under_test_pos.y + placeDiagRange};
            place_worm(true, 3, enemy_worm_pos1, state);

            check_shot_hit(state, 2, ShootCommand::ShootDirection::NE, true);
            check_shot_hit(state, 3, ShootCommand::ShootDirection::SW, true);
        }

        THEN("Shooting friendly worms 3")
        {
            Position enemy_worm_pos2{worm_under_test_pos.x, worm_under_test_pos.y + placeRange};
            place_worm(true, 2, enemy_worm_pos2, state);
            Position enemy_worm_pos3{worm_under_test_pos.x + placeDiagRange, worm_under_test_pos.y + placeDiagRange};
            place_worm(true, 3, enemy_worm_pos3, state);

            check_shot_hit(state, 2, ShootCommand::ShootDirection::S, true);
            check_shot_hit(state, 3, ShootCommand::ShootDirection::SE, true);
        }

        THEN("Shooting friendly worms 4")
        {
            Position enemy_worm_pos1{worm_under_test_pos.x - placeRange, worm_under_test_pos.y};
            place_worm(true, 2, enemy_worm_pos1, state);
            Position enemy_worm_pos2{worm_under_test_pos.x + placeRange, worm_under_test_pos.y};
            place_worm(true, 3, enemy_worm_pos2, state);

            check_shot_hit(state, 2, ShootCommand::ShootDirection::W, true);
            check_shot_hit(state, 3, ShootCommand::ShootDirection::E, true);
        }
    }
}

TEST_CASE( "Shoot command miss far away", "[Shoot_command][Shot_missed]" ) {

    GIVEN("A game state and a shoot command")
    {
        auto state = std::make_shared<GameState>();

        Position worm_under_test_pos{15,15};
        place_worm(true, 1, worm_under_test_pos, state);

        auto range = GameConfig::commandoWorms.weapon.range + 1;
        auto diagRange = GameConfig::commandoWorms.weapon.diagRange + 1;

        THEN("Shooting hits worms far away N")
        {
            Position enemy_worm_pos1{worm_under_test_pos.x - diagRange, worm_under_test_pos.y - diagRange};
            place_worm(false, 1, enemy_worm_pos1, state);
            Position enemy_worm_pos2{worm_under_test_pos.x, worm_under_test_pos.y - range};
            place_worm(false, 2, enemy_worm_pos2, state);
            Position enemy_worm_pos3{worm_under_test_pos.x + diagRange, worm_under_test_pos.y - diagRange};
            place_worm(false, 3, enemy_worm_pos3, state);

            check_shot_missed(state, ShootCommand::ShootDirection::NW);
            check_shot_missed(state, ShootCommand::ShootDirection::N);
            check_shot_missed(state, ShootCommand::ShootDirection::NE);
        }

        THEN("Shooting hits worms far away S")
        {
            Position enemy_worm_pos1{worm_under_test_pos.x - diagRange, worm_under_test_pos.y + diagRange};
            place_worm(false, 1, enemy_worm_pos1, state);
            Position enemy_worm_pos2{worm_under_test_pos.x, worm_under_test_pos.y + range};
            place_worm(false, 2, enemy_worm_pos2, state);
            Position enemy_worm_pos3{worm_under_test_pos.x + diagRange, worm_under_test_pos.y + diagRange};
            place_worm(false, 3, enemy_worm_pos3, state);

            check_shot_missed(state, ShootCommand::ShootDirection::SW);
            check_shot_missed(state, ShootCommand::ShootDirection::S);
            check_shot_missed(state, ShootCommand::ShootDirection::SE);
        }

        THEN("Shooting hits worms far away EW")
        {
            Position enemy_worm_pos1{worm_under_test_pos.x - range, worm_under_test_pos.y};
            place_worm(false, 1, enemy_worm_pos1, state);
            Position enemy_worm_pos2{worm_under_test_pos.x + range, worm_under_test_pos.y};
            place_worm(false, 2, enemy_worm_pos2, state);

            check_shot_missed(state, ShootCommand::ShootDirection::W);
            check_shot_missed(state, ShootCommand::ShootDirection::E);
        }
    }
}

TEST_CASE( "Shoot command miss shadows", "[Shoot_command][Shot_missed]" ) {

    GIVEN("A game state and a shoot command")
    {
        auto state = std::make_shared<GameState>();

        Position worm_under_test_pos{15,15};
        place_worm(true, 1, worm_under_test_pos, state);

        auto range = GameConfig::commandoWorms.weapon.range;
        auto diagRange = GameConfig::commandoWorms.weapon.diagRange;

        THEN("Shooting hits worms far away N")
        {
            Position enemy_worm_pos1{worm_under_test_pos.x - diagRange + 1, worm_under_test_pos.y - diagRange};
            place_worm(false, 1, enemy_worm_pos1, state);
            Position enemy_worm_pos2{worm_under_test_pos.x + 1, worm_under_test_pos.y - range};
            place_worm(false, 2, enemy_worm_pos2, state);
            Position enemy_worm_pos3{worm_under_test_pos.x + diagRange - 1, worm_under_test_pos.y - diagRange};
            place_worm(false, 3, enemy_worm_pos3, state);

            check_shot_missed(state, ShootCommand::ShootDirection::NW);
            check_shot_missed(state, ShootCommand::ShootDirection::N);
            check_shot_missed(state, ShootCommand::ShootDirection::NE);
        }

        THEN("Shooting hits worms far away S")
        {
            Position enemy_worm_pos1{worm_under_test_pos.x - diagRange + 1, worm_under_test_pos.y + diagRange};
            place_worm(false, 1, enemy_worm_pos1, state);
            Position enemy_worm_pos2{worm_under_test_pos.x + 1, worm_under_test_pos.y + range};
            place_worm(false, 2, enemy_worm_pos2, state);
            Position enemy_worm_pos3{worm_under_test_pos.x + diagRange - 1, worm_under_test_pos.y + diagRange};
            place_worm(false, 3, enemy_worm_pos3, state);

            check_shot_missed(state, ShootCommand::ShootDirection::SW);
            check_shot_missed(state, ShootCommand::ShootDirection::S);
            check_shot_missed(state, ShootCommand::ShootDirection::SE);
        }

        THEN("Shooting hits worms far away EW")
        {
            Position enemy_worm_pos1{worm_under_test_pos.x - range, worm_under_test_pos.y+1};
            place_worm(false, 1, enemy_worm_pos1, state);
            Position enemy_worm_pos2{worm_under_test_pos.x + range, worm_under_test_pos.y-1};
            place_worm(false, 2, enemy_worm_pos2, state);

            check_shot_missed(state, ShootCommand::ShootDirection::W);
            check_shot_missed(state, ShootCommand::ShootDirection::E);
        }
    }
}

TEST_CASE( "Shoot command obstacles : dirt", "[Shoot_command][Shot_missed]" ) {

    GIVEN("A game state and a shoot command")
    {
        auto state = std::make_shared<GameState>();

        Position worm_under_test_pos{15,15};
        place_worm(true, 1, worm_under_test_pos, state);

        //surround the dude in dirt
        state->map[worm_under_test_pos.x - 1][worm_under_test_pos.y - 1].type = CellType::DIRT;
        state->map[worm_under_test_pos.x]    [worm_under_test_pos.y - 1].type = CellType::DIRT;
        state->map[worm_under_test_pos.x + 1][worm_under_test_pos.y - 1].type = CellType::DIRT;

        state->map[worm_under_test_pos.x - 1][worm_under_test_pos.y + 1].type = CellType::DIRT;
        state->map[worm_under_test_pos.x]    [worm_under_test_pos.y + 1].type = CellType::DIRT;
        state->map[worm_under_test_pos.x + 1][worm_under_test_pos.y + 1].type = CellType::DIRT;

        state->map[worm_under_test_pos.x - 1][worm_under_test_pos.y].type = CellType::DIRT;
        state->map[worm_under_test_pos.x + 1][worm_under_test_pos.y].type = CellType::DIRT;

        auto range = GameConfig::commandoWorms.weapon.range;
        auto diagRange = GameConfig::commandoWorms.weapon.diagRange;

        THEN("Shooting hits worms far away N")
        {
            Position enemy_worm_pos1{worm_under_test_pos.x - diagRange + 1, worm_under_test_pos.y - diagRange};
            place_worm(false, 1, enemy_worm_pos1, state);
            Position enemy_worm_pos2{worm_under_test_pos.x + 1, worm_under_test_pos.y - range};
            place_worm(false, 2, enemy_worm_pos2, state);
            Position enemy_worm_pos3{worm_under_test_pos.x + diagRange - 1, worm_under_test_pos.y - diagRange};
            place_worm(false, 3, enemy_worm_pos3, state);

            check_shot_missed(state, ShootCommand::ShootDirection::NW);
            check_shot_missed(state, ShootCommand::ShootDirection::N);
            check_shot_missed(state, ShootCommand::ShootDirection::NE);
        }

        THEN("Shooting hits worms far away S")
        {
            Position enemy_worm_pos1{worm_under_test_pos.x - diagRange + 1, worm_under_test_pos.y + diagRange};
            place_worm(false, 1, enemy_worm_pos1, state);
            Position enemy_worm_pos2{worm_under_test_pos.x + 1, worm_under_test_pos.y + range};
            place_worm(false, 2, enemy_worm_pos2, state);
            Position enemy_worm_pos3{worm_under_test_pos.x + diagRange - 1, worm_under_test_pos.y + diagRange};
            place_worm(false, 3, enemy_worm_pos3, state);

            check_shot_missed(state, ShootCommand::ShootDirection::SW);
            check_shot_missed(state, ShootCommand::ShootDirection::S);
            check_shot_missed(state, ShootCommand::ShootDirection::SE);
        }

        THEN("Shooting hits worms far away EW")
        {
            Position enemy_worm_pos1{worm_under_test_pos.x - range, worm_under_test_pos.y+1};
            place_worm(false, 1, enemy_worm_pos1, state);
            Position enemy_worm_pos2{worm_under_test_pos.x + range, worm_under_test_pos.y-1};
            place_worm(false, 2, enemy_worm_pos2, state);

            check_shot_missed(state, ShootCommand::ShootDirection::W);
            check_shot_missed(state, ShootCommand::ShootDirection::E);
        }
    }
}

TEST_CASE( "Shoot command obstacles : other worms", "[Shoot_command][Shot_missed]" ) {

    GIVEN("A game state and a shoot command")
    {
        auto state = std::make_shared<GameState>();

        Position worm_under_test_pos{15,15};
        place_worm(true, 1, worm_under_test_pos, state);

        auto range = GameConfig::commandoWorms.weapon.range;
        auto diagRange = GameConfig::commandoWorms.weapon.diagRange;

        THEN("Shooting misses target worms 1")
        {
            //place enemies far away
            Position enemy_worm_pos1{worm_under_test_pos.x - diagRange, worm_under_test_pos.y - diagRange};
            place_worm(false, 1, enemy_worm_pos1, state);
            Position enemy_worm_pos2{worm_under_test_pos.x, worm_under_test_pos.y - range};
            place_worm(false, 2, enemy_worm_pos2, state);

            //place friendlies between
            Position friendly_worm_pos1{worm_under_test_pos.x - 1, worm_under_test_pos.y - 1};
            place_worm(true, 1, enemy_worm_pos1, state);
            Position friendly_worm_pos2{worm_under_test_pos.x, worm_under_test_pos.y - 1};
            place_worm(true, 2, enemy_worm_pos2, state);

            //check we miss the outer worms
            CHECK(!shot_hits(state, 1, ShootCommand::ShootDirection::NW));
            CHECK(!shot_hits(state, 2, ShootCommand::ShootDirection::N));
        }

        THEN("Shooting misses target worms 2")
        {
            //place enemies far away
            Position enemy_worm_pos1{worm_under_test_pos.x + diagRange, worm_under_test_pos.y + diagRange};
            place_worm(false, 1, enemy_worm_pos1, state);
            Position enemy_worm_pos2{worm_under_test_pos.x - diagRange, worm_under_test_pos.y - diagRange};
            place_worm(false, 2, enemy_worm_pos2, state);

            //place friendlies between
            Position friendly_worm_pos1{worm_under_test_pos.x + 1, worm_under_test_pos.y + 1};
            place_worm(true, 1, enemy_worm_pos1, state);
            Position friendly_worm_pos2{worm_under_test_pos.x - 1, worm_under_test_pos.y - 1};
            place_worm(true, 2, enemy_worm_pos2, state);

            //check we miss the outer worms
            CHECK(!shot_hits(state, 1, ShootCommand::ShootDirection::NE));
            CHECK(!shot_hits(state, 2, ShootCommand::ShootDirection::SW));
        }

        THEN("Shooting misses target worms 3")
        {
            //place enemies far away
            Position enemy_worm_pos1{worm_under_test_pos.x, worm_under_test_pos.y + range};
            place_worm(false, 1, enemy_worm_pos1, state);
            Position enemy_worm_pos2{worm_under_test_pos.x + range, worm_under_test_pos.y + range};
            place_worm(false, 2, enemy_worm_pos2, state);

            //place friendlies between
            Position friendly_worm_pos1{worm_under_test_pos.x, worm_under_test_pos.y + 1};
            place_worm(true, 1, enemy_worm_pos1, state);
            Position friendly_worm_pos2{worm_under_test_pos.x + 1, worm_under_test_pos.y + 1};
            place_worm(true, 2, enemy_worm_pos2, state);

            //check we miss the outer worms
            CHECK(!shot_hits(state, 1, ShootCommand::ShootDirection::S));
            CHECK(!shot_hits(state, 2, ShootCommand::ShootDirection::SE));
        }

        THEN("Shooting misses target worms 4")
        {
            //place enemies far away
            Position enemy_worm_pos1{worm_under_test_pos.x - range, worm_under_test_pos.y};
            place_worm(false, 1, enemy_worm_pos1, state);
            Position enemy_worm_pos2{worm_under_test_pos.x + range, worm_under_test_pos.y};
            place_worm(false, 2, enemy_worm_pos2, state);

            //place friendlies between
            Position friendly_worm_pos1{worm_under_test_pos.x - 1, worm_under_test_pos.y};
            place_worm(true, 1, enemy_worm_pos1, state);
            Position friendly_worm_pos2{worm_under_test_pos.x + 1, worm_under_test_pos.y};
            place_worm(true, 2, enemy_worm_pos2, state);

            //check we miss the outer worms
            CHECK(!shot_hits(state, 1, ShootCommand::ShootDirection::W));
            CHECK(!shot_hits(state, 2, ShootCommand::ShootDirection::E));
        }
    }
}

//TODO check correct behaviour when 2 worms shoot the same guy in the same turn

//TODO check WormRoundProcessorTest.kt