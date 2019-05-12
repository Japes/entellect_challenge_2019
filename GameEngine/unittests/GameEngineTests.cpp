#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"
#include "../GameEngine.hpp"
#include "AllCommands.hpp"

TEST_CASE( "I can make a game engine instance", "[sanity]" ) {
    GameEngine eng;
    REQUIRE( true );
}

TEST_CASE( "Commands are resolved in the right order", "[command_order]" ) {
    //according to the rules:
    //move
    //dig
    //shoot

    //seems to be this order in the engine:
    //dig
    //move
    //shoot
}

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

TEST_CASE( "Move command", "[Move_command]" ) {

    GIVEN("A game state and a move command")
    {
        auto state = std::make_shared<GameState>();

        state->player1.GetCurrentWorm()->position = {10,10};
        state->map[10][10].worm = state->player1.GetCurrentWorm();
        state->map[11][10].type = CellType::DIRT;
        state->map[11][11].type = CellType::DIRT;
        state->map[10][11].type = CellType::DEEP_SPACE;
        GameEngine eng(state);

        int expectedDoNothings = 0;
        REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);

        TeleportCommand player1move(true, state, {0,0});
        TeleportCommand player2move(false, state, {0,0});

        THEN("out of bounds (too low) is invalid")
        {
            player1move = TeleportCommand(true, state, {-1,-1});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("out of bounds (too high) is invalid")
        {
            player1move = TeleportCommand(true, state, {200,200});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("too far (really far) away is invalid")
        {
            player1move = TeleportCommand(true, state, {1,1});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("too far away (just out of range) is invalid")
        {
            player1move = TeleportCommand(true, state, {10,12});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("too far away is invalid")
        {
            player1move = TeleportCommand(true, state, {8,8});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("dirt is invalid")
        {
            player1move = TeleportCommand(true, state, {11,10});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }

        THEN("deep space is invalid")
        {
            player1move = TeleportCommand(true, state, {10,11});    eng.AdvanceState(player1move,player2move);    ++expectedDoNothings;
            REQUIRE(state->player1.consecutiveDoNothingCount == expectedDoNothings);
        }
    }
}

/*
    fun test_apply_valid() {
        val startingPosition = Point(0, 0)
        val targetPosition = Point(1, 1)

        val testCommand = TeleportCommand(targetPosition, Random, config)
        val worm = CommandoWorm.build(0, config, Point(0, 0))
        val player = WormsPlayer.build(0, listOf(worm), config)
        val testMap = buildMapWithCellType(listOf(player), 2, CellType.AIR)

        assertTrue(testCommand.validate(testMap, worm).isValid)
        testCommand.execute(testMap, worm)

        assertEquals(testCommand.target, worm.position)
        assertEquals(testMap[testCommand.target].occupier, worm)
        assertEquals(0, worm.roundMoved)
        assertEquals(startingPosition, worm.previousPosition)
    }

    fun test_apply_twice() {
        val startingPosition = Point(0, 0)
        val targetPosition = Point(1, 1)

        val testCommand = TeleportCommand(targetPosition, Random, config)
        val worm = CommandoWorm.build(0, config, startingPosition)
        val player = WormsPlayer.build(0, listOf(worm), config)
        val testMap = buildMapWithCellType(listOf(player), 2, CellType.AIR)

        assertTrue(testCommand.validate(testMap, worm).isValid)
        testCommand.execute(testMap, worm)

        assertEquals(testCommand.target, worm.position)
        assertEquals(testMap[testCommand.target].occupier, worm)
        assertEquals(0, worm.roundMoved)
        assertEquals(startingPosition, worm.previousPosition)

        assertFalse(testCommand.validate(testMap, worm).isValid)
    }

    fun test_apply_nonEmpty() {
        val testCommand = TeleportCommand(1, 1, Random, config)
        val worm = CommandoWorm.build(0, config, Point(0, 0))
        val player = WormsPlayer.build(0, listOf(worm), config)
        val testMap = buildMapWithCellType(listOf(player), 2, CellType.AIR)

        testMap[1, 1].occupier = CommandoWorm.build(0, config, Point(0, 0))

        assertFalse(testCommand.validate(testMap, worm).isValid)
    }
*/
    /**
     * When two worms move to the same cell in the same round
    fun test_apply_collide_pushback() {
        val random: Random = mock {
            on { nextBoolean() } doReturn true
        }

        val testCommand = TeleportCommand(Point(1, 1), random, config)
        val wormA = CommandoWorm.build(0, config, Point(0, 0))
        val wormB = CommandoWorm.build(0, config, Point(2, 1))
        val player = WormsPlayer.build(0, listOf(wormA, wormB), config)
        val testMap = buildMapWithCellType(listOf(player), 4, CellType.AIR)

        assertTrue(testCommand.validate(testMap, wormA).isValid, "Command A Valid")
        testCommand.execute(testMap, wormA)
        assertTrue(testCommand.validate(testMap, wormB).isValid, "Command B Valid")
        testCommand.execute(testMap, wormB)

        assertFalse(testMap[1, 1].isOccupied(), "Target not occupied")
        assertTrue(testMap[0, 0].isOccupied())
        assertTrue(testMap[2, 1].isOccupied())
        assertEquals(wormA, testMap[0, 0].occupier)
        assertEquals(wormB, testMap[2, 1].occupier)
    }

    /**
     * When two worms move to the same cell in the same round
    @Test
    fun test_apply_collide_swap() {
        val random: Random = mock {
            on { nextBoolean() } doReturn false
        }

        val testCommand = TeleportCommand(Point(1, 1), random, config)

        val wormA = CommandoWorm.build(0, config, Point(0, 0))
        val wormB = CommandoWorm.build(0, config, Point(2, 1))
        val playerA = WormsPlayer.build(0, listOf(wormA, wormB), config)
        val testMap = buildMapWithCellType(listOf(playerA), 3, CellType.AIR)

        assertTrue(testCommand.validate(testMap, wormA).isValid, "Command A Valid")
        testCommand.execute(testMap, wormA)
        assertTrue(testCommand.validate(testMap, wormB).isValid, "Command B Valid")
        testCommand.execute(testMap, wormB)

        assertFalse(testMap[1, 1].isOccupied(), "Target not occupied")
        assertTrue(testMap[0, 0].isOccupied())
        assertTrue(testMap[2, 1].isOccupied())

        assertEquals(wormB, testMap[0, 0].occupier)
        assertEquals(wormA, testMap[2, 1].occupier)
    }*/

//check all command types

TEST_CASE( "Active worms are chosen correctly", "[active_worm]" ) {
    //test when worms have died
}

TEST_CASE( "12 do nothings means disqualified", "[disqualified]" ) {
    //check for player 1 and player 2
}

TEST_CASE( "Points are allocated correctly", "[command_order]" ) {
    //check for each thing mentioned in the rules and confirm game engine concurs
}

TEST_CASE( "Comparison with jave engine", "[comparison]" ) {
    //read a bunch of known state files, moves, and expected outputs generated by their engine.
    //read in those files, apply the moves, and compare result with theirs.
}
