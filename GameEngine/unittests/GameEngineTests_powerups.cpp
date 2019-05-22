#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "GameEngineTestUtils.hpp"

TEST_CASE( "Healthpack", "[powerup]" ) {

    GIVEN("A game state with a worm next to a powerup")
    {
        auto state = std::make_shared<GameState>();
        Position worm_under_test_pos{10,10};
        Worm* worm = place_worm(true, 1, worm_under_test_pos, state);
        Position powerup_pos{10,10};
        place_powerup(powerup_pos, state);
        GameEngine eng(state);

        THEN("A healthpack can resurrect a worm that is still on the map (i.e. it died in this round")
        {
            worm->health = 0;
            TeleportCommand player1move(true, state, powerup_pos);
            DoNothingCommand player2move(false, state);
            eng.AdvanceState(player1move, player2move);

            REQUIRE(worm->health == GameConfig::healthPackHp);
            REQUIRE(worm == state->Cell_at(powerup_pos)->worm);
            REQUIRE(state->Cell_at(powerup_pos)->powerup == nullptr);
            REQUIRE(!worm->IsDead());
        }
    }
}

    //this test is in the kotlin engine, not sure i understand it
    //of course a worm wont be revived if it's not on the same spot as the powerup?
    
    /*
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
