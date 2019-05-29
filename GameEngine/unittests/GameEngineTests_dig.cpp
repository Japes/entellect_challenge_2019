#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"

TEST_CASE( "Dig command", "[Dig_command]" ) {

    GIVEN("A game state and a dig command")
    {
        auto state = std::make_shared<GameState>();

        state->player1.GetCurrentWorm()->position = {10,10};
        state->map[11][10].type = CellType::DIRT;
        state->map[11][11].type = CellType::DIRT;
        state->map[10][11].type = CellType::DEEP_SPACE;
        GameEngine eng(state);

        int expectedDoNothings = 0;
        REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);

        DigCommand player1move(true, state, {0,0});
        DigCommand player2move(false, state, {0,0});

        THEN("out of bounds (too low) is invalid")
        {
            player1move = DigCommand(true, state, {-11,-1});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("out of bounds (too high) is invalid")
        {
            player1move = DigCommand(true, state, {200,200});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("too far away is invalid")
        {
            player1move = DigCommand(true, state, {1,1});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("too far away is invalid")
        {
            player1move = DigCommand(true, state, {10,12});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("empty space is invalid")
        {
            player1move = DigCommand(true, state, {10,9});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("deep space is invalid")
        {
            player1move = DigCommand(true, state, {10,11});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("digging dirt is valid")
        {
            player1move = DigCommand(true, state, {11,10});    eng.AdvanceState(player1move,player2move);    //++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
            REQUIRE(state->map[11][10].type == CellType::AIR);
        }

        THEN("digging dirt diagonally is valid")
        {
            player1move = DigCommand(true, state, {11,11});    eng.AdvanceState(player1move,player2move);    //++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
            REQUIRE(state->map[11][11].type == CellType::AIR);
        }
    }
}

TEST_CASE( "Get dig string", "[Dig_string]" ) {
    auto state = std::make_shared<GameState>();
    DigCommand move(true, state, {11,11});
    REQUIRE(move.GetCommandString() == "dig 11 11");

    DigCommand move1(true, state, {0,31});
    REQUIRE(move1.GetCommandString() == "dig 0 31");
}

//Two worms digging the same cell in the same turn is a valid move
/*
    fun processRound_digSameHole() {
        val player1 = WormsPlayer.build(1, listOf(CommandoWorm.build(0, config, Point(0, 0))), config)
        val player2 = WormsPlayer.build(2, listOf(CommandoWorm.build(0, config, Point(2, 2))), config)
        val map = buildMapWithCellType(listOf(player1, player2), 3, CellType.DIRT)
        val command = DigCommand(1, 1, TEST_CONFIG)

        val commandMap = mapOf(Pair(player1, command), Pair(player2, command))
        roundProcessor.processRound(map, commandMap)

        assertEquals(CellType.AIR, map[1, 1].type)
    }

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
