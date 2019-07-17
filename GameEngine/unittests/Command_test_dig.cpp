#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "GameEngineTestUtils.hpp"

TEST_CASE( "Dig command", "[Dig_command]" ) {

    GIVEN("A game state and a dig command")
    {
        auto state = std::make_shared<GameState>();

        place_worm(true, 1, {10,10}, state);
        place_worm(false, 1, {12,12}, state);
        state->SetCellTypeAt({11, 10}, CellType::DIRT);
        state->SetCellTypeAt({11, 11}, CellType::DIRT);
        state->SetCellTypeAt({10, 11}, CellType::DEEP_SPACE);
        GameEngine eng(state);

        int expectedDoNothings = 0;
        REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);

        DigCommand player1move({0,0});
        DigCommand player2move({0,0});

        THEN("out of bounds (too low) is invalid")
        {
            player1move = DigCommand({-11,-1});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("out of bounds (too high) is invalid")
        {
            player1move = DigCommand({200,200});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("too far away is invalid")
        {
            player1move = DigCommand({1,1});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("too far away is invalid")
        {
            player1move = DigCommand({10,12});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("empty space is invalid")
        {
            player1move = DigCommand({10,9});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("deep space is invalid")
        {
            player1move = DigCommand({10,11});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("digging dirt is valid")
        {
            player1move = DigCommand({11,10});    eng.AdvanceState(player1move,player2move);    //++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
            REQUIRE(state->CellType_at({11, 10}) == CellType::AIR);
        }

        THEN("digging dirt diagonally is valid")
        {
            player1move = DigCommand({11,11});    eng.AdvanceState(player1move,player2move);    //++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
            REQUIRE(state->CellType_at({11, 11}) == CellType::AIR);
        }

        THEN("Both worms digging same hole is valid")
        {
            player1move = DigCommand({11,11});    
            player2move = DigCommand({11,11});    
            eng.AdvanceState(player1move,player2move);    
            //++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
            REQUIRE(state->player2.consecutiveDoNothingCount == expectedDoNothings);
            REQUIRE(state->CellType_at({11, 11}) == CellType::AIR);
        }
    }
}

TEST_CASE( "Get dig string", "[Dig_string]" ) {
    auto state = std::make_shared<GameState>();
    DigCommand move({11,11});
    REQUIRE(move.GetCommandString() == "dig 11 11");

    DigCommand move1({0,31});
    REQUIRE(move1.GetCommandString() == "dig 0 31");
}

/*

    @Test
    fun processRound_moveDigSameLocation() {
        val player1 = WormsPlayer.build(1, listOf(CommandoWorm.build(0, config, Point(0, 0))), config)
        val player2 = WormsPlayer.build(2, listOf(CommandoWorm.build(0, config, Point(2, 2))), config)
        val map = buildMapWithCellType(listOf(player1, player2), 3, CellType.DIRT)
        val digCommand = DigCommand(1, 1, TEST_CONFIG)
        val moveCommand = TeleportCommand(Point(1, 1), random, TEST_CONFIG)

        val commandMap = mapOf(Pair(player1, digCommand), Pair(player2, moveCommand))
        roundProcessor.processRound(map, commandMap)

        assertNull(map[1, 1].occupier)
        assertEquals(CellType.AIR, map[1, 1].type)
        assertEquals(player1.worms[0], map[0, 0].occupier)
        assertEquals(player2.worms[0], map[2, 2].occupier)
        assertEquals(1, map.currentRoundErrors.size)
    }


    @Test
    fun processRound_shootDigOpen() {
        val player1 = WormsPlayer.build(1, listOf(CommandoWorm.build(0, config, Point(0, 0))), config)
        val player2 = WormsPlayer.build(2, listOf(CommandoWorm.build(0, config, Point(2, 2))), config)
        val map = buildMapWithCellType(listOf(player1, player2), 3, CellType.DIRT)
        map[0, 0].type = CellType.AIR

        val digCommand = DigCommand(1, 1, TEST_CONFIG)
        val shootCommand = ShootCommand(Direction.UP_LEFT, TEST_CONFIG)

        val commandMap = mapOf(Pair(player1, digCommand), Pair(player2, shootCommand))
        roundProcessor.processRound(map, commandMap)

        assertNull(map[1, 1].occupier)
        assertEquals(CellType.AIR, map[1, 1].type)
        assertEquals(config.commandoWorms.initialHp, player2.worms[0].health)
        assertNotEquals(config.commandoWorms.initialHp, player1.worms[0].health)
    }
*/
