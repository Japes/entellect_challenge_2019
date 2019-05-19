#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"

TEST_CASE( "Healthpack", "[powerup]" ) {

/*
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
    */
}

/*

    // A healthpack can resurrect a worm that is still on the map (i.e. it died in this round)
    @Test
    fun processRound_applyHealthpackResurrection() {
        val worm = CommandoWorm.build(0, config, Point(2, 2))
        worm.health = 0
        val player = WormsPlayer.build(0, listOf(worm), config)
        val map = buildMapWithCellType(listOf(player), 5, CellType.AIR)

        val target = Point(1, 2)
        val command = TeleportCommand(target, Random, config)
        map[target].powerup = HealthPack(config.healthPackHp)

        command.execute(map, worm)
        roundProcessor.processRound(map, mapOf(Pair(player, command)))

        assertEquals(config.healthPackHp, player.health)
        assertEquals(worm, map[target].occupier)
        assertNull(map[target].powerup)
        assertFalse(player.dead)
    }

     // A healthpack cannot resurrect a worm that is no longer on the map (i.e. it died in a previous round)
    @Test
    fun  processRound_applyHealthpackNoResurrection() {
        val worm = CommandoWorm.build(0, config, Point(2, 2))
        worm.health = 0
        val player = WormsPlayer.build(0, listOf(worm), config)
        val map = buildMapWithCellType(listOf(player), 5, CellType.AIR)

        val target = Point(1, 2)
        map[target].powerup = HealthPack(config.healthPackHp)

        roundProcessor.processRound(map, emptyMap())

        assertEquals(0, player.health)
        assertNull(map[2, 2].occupier)
        assertNotNull(map[target].powerup)
        assertTrue(player.dead)
    }

    */
