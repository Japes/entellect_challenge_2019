#include "catch.hpp"
#include "../GameEngine.hpp"
#include "../GameConfig.hpp"
#include "AllCommands.hpp"
#include "GameEngineTestUtils.hpp"
#include "Utilities.hpp"
#include "../GameState/GameStateLoader.hpp"
#include "../../Bot/Bot.hpp"
#include "../GameEngine/NextTurn.hpp"
#include "Evaluators.hpp"

TEST_CASE( "AdjustOpponentSpellCount", "[AdjustOpponentSpellCount]" ) {
    GIVEN("game states before/after throwing a snowball and a Bot")
    {
        auto roundJSON = Utilities::ReadJsonFile("./Test_files/spellCounts/beforeSnowBall.json");
        GameState stateBeforeThrow = GameStateLoader::LoadGameState(roundJSON);

        auto round2JSON = Utilities::ReadJsonFile("./Test_files/spellCounts/afterSnowBall.json");
        GameState stateAfterThrow = GameStateLoader::LoadGameState(round2JSON);

        int playThroughDepth{24};
        int nodeDepth{1};
        int dirtsForBanana{10};
        int clearSpaceForHeuristic{-1}; //if everything is clear for this distance, use heuristic
        bool patternDetectEnable{false};
        uint64_t mcTime_ns{880000000};
        float mc_c{std::sqrt(2)};
        int mc_runsBeforeClockCheck{50};

        GetEvaluatorFn_t eval = [&](bool, GameStatePtr){ return Evaluators::RushHealth; };

        Bot bot(eval,
                    playThroughDepth, nodeDepth, 
                    dirtsForBanana, clearSpaceForHeuristic, patternDetectEnable, NextTurn::WormCanShoot, 
                    mcTime_ns, mc_c, mc_runsBeforeClockCheck);

        THEN("snowball counts are as we expect")
        {
            REQUIRE(stateBeforeThrow.player2.worms[0].snowball_count == 0);
            REQUIRE(stateBeforeThrow.player1.worms[0].snowball_count == 0);
            REQUIRE(stateBeforeThrow.player2.worms[1].snowball_count == 0);
            REQUIRE(stateBeforeThrow.player1.worms[1].snowball_count == 0);
            REQUIRE(stateBeforeThrow.player2.worms[2].snowball_count == GameConfig::technologistWorms.snowball.count);
            REQUIRE(stateBeforeThrow.player1.worms[2].snowball_count == GameConfig::technologistWorms.snowball.count);

            REQUIRE(stateAfterThrow.player2.worms[0].snowball_count == 0);
            REQUIRE(stateAfterThrow.player1.worms[0].snowball_count == 0);
            REQUIRE(stateAfterThrow.player2.worms[1].snowball_count == 0);
            REQUIRE(stateAfterThrow.player1.worms[1].snowball_count == 0);
            REQUIRE(stateAfterThrow.player2.worms[2].snowball_count == GameConfig::technologistWorms.snowball.count);
            REQUIRE(stateAfterThrow.player1.worms[2].snowball_count == GameConfig::technologistWorms.snowball.count);
        }

        WHEN("We update the spell count")
        {
            bot.AdjustOpponentSpellCount(true, &stateAfterThrow, &stateBeforeThrow);
            THEN("Snowball count goes down by 1")
            {
                REQUIRE(stateBeforeThrow.player2.worms[0].snowball_count == 0);
                REQUIRE(stateBeforeThrow.player1.worms[0].snowball_count == 0);
                REQUIRE(stateBeforeThrow.player2.worms[1].snowball_count == 0);
                REQUIRE(stateBeforeThrow.player1.worms[1].snowball_count == 0);
                REQUIRE(stateBeforeThrow.player2.worms[2].snowball_count == GameConfig::technologistWorms.snowball.count);
                REQUIRE(stateBeforeThrow.player1.worms[2].snowball_count == GameConfig::technologistWorms.snowball.count);

                REQUIRE(stateAfterThrow.player2.worms[0].snowball_count == 0);
                REQUIRE(stateAfterThrow.player1.worms[0].snowball_count == 0);
                REQUIRE(stateAfterThrow.player2.worms[1].snowball_count == 0);
                REQUIRE(stateAfterThrow.player1.worms[1].snowball_count == 0);
                REQUIRE(stateAfterThrow.player2.worms[2].snowball_count == GameConfig::technologistWorms.snowball.count - 1);
                REQUIRE(stateAfterThrow.player1.worms[2].snowball_count == GameConfig::technologistWorms.snowball.count);
            }
        }
    }

    GIVEN("game states before/after throwing a banana and a Bot")
    {
        auto roundJSON = Utilities::ReadJsonFile("./Test_files/spellCounts/beforeBanana.json");
        GameState stateBeforeThrow = GameStateLoader::LoadGameState(roundJSON);

        auto round2JSON = Utilities::ReadJsonFile("./Test_files/spellCounts/afterBanana.json");
        GameState stateAfterThrow = GameStateLoader::LoadGameState(round2JSON);

        int playThroughDepth{24};
        int nodeDepth{1};
        int dirtsForBanana{10};
        int clearSpaceForHeuristic{-1}; //if everything is clear for this distance, use heuristic
        bool patternDetectEnable{false};
        uint64_t mcTime_ns{880000000};
        float mc_c{std::sqrt(2)};
        int mc_runsBeforeClockCheck{50};

        GetEvaluatorFn_t eval = [&](bool, GameStatePtr){ return Evaluators::RushHealth; };

        Bot bot(eval,
                    playThroughDepth, nodeDepth, 
                    dirtsForBanana, clearSpaceForHeuristic, patternDetectEnable, NextTurn::WormCanShoot,
                    mcTime_ns, mc_c, mc_runsBeforeClockCheck);
        

        THEN("Banana counts are as we expect")
        {
            REQUIRE(stateBeforeThrow.player2.worms[0].banana_bomb_count == 0);
            REQUIRE(stateBeforeThrow.player1.worms[0].banana_bomb_count == 0);
            REQUIRE(stateBeforeThrow.player2.worms[1].banana_bomb_count == GameConfig::agentWorms.banana.count);
            REQUIRE(stateBeforeThrow.player1.worms[1].banana_bomb_count == GameConfig::agentWorms.banana.count);
            REQUIRE(stateBeforeThrow.player2.worms[2].banana_bomb_count == 0);
            REQUIRE(stateBeforeThrow.player1.worms[2].banana_bomb_count == 0);

            REQUIRE(stateAfterThrow.player2.worms[0].banana_bomb_count == 0);
            REQUIRE(stateAfterThrow.player1.worms[0].banana_bomb_count == 0);
            REQUIRE(stateAfterThrow.player2.worms[1].banana_bomb_count == GameConfig::agentWorms.banana.count);
            REQUIRE(stateAfterThrow.player1.worms[1].banana_bomb_count == GameConfig::agentWorms.banana.count);
            REQUIRE(stateAfterThrow.player2.worms[2].banana_bomb_count == 0);
            REQUIRE(stateAfterThrow.player1.worms[2].banana_bomb_count == 0);
        }

        WHEN("We update the spell count")
        {
            bot.AdjustOpponentSpellCount(true, &stateAfterThrow, &stateBeforeThrow);
            THEN("Banana count goes down by 1")
            {
                REQUIRE(stateBeforeThrow.player2.worms[0].banana_bomb_count == 0);
                REQUIRE(stateBeforeThrow.player1.worms[0].banana_bomb_count == 0);
                REQUIRE(stateBeforeThrow.player2.worms[1].banana_bomb_count == GameConfig::agentWorms.banana.count);
                REQUIRE(stateBeforeThrow.player1.worms[1].banana_bomb_count == GameConfig::agentWorms.banana.count);
                REQUIRE(stateBeforeThrow.player2.worms[2].banana_bomb_count == 0);
                REQUIRE(stateBeforeThrow.player1.worms[2].banana_bomb_count == 0);

                REQUIRE(stateAfterThrow.player2.worms[0].banana_bomb_count == 0);
                REQUIRE(stateAfterThrow.player1.worms[0].banana_bomb_count == 0);
                REQUIRE(stateAfterThrow.player2.worms[1].banana_bomb_count == GameConfig::agentWorms.banana.count - 1);
                REQUIRE(stateAfterThrow.player1.worms[1].banana_bomb_count == GameConfig::agentWorms.banana.count);
                REQUIRE(stateAfterThrow.player2.worms[2].banana_bomb_count == 0);
                REQUIRE(stateAfterThrow.player1.worms[2].banana_bomb_count == 0);
            }
        }
    }
}

TEST_CASE( "Basic sanity", "[.BotSanity]" ) {
    GIVEN("a Bot")
    {
        auto roundJSON = Utilities::ReadJsonFile("./Test_files/spellCounts/beforeSnowBall.json");
        auto round2JSON = Utilities::ReadJsonFile("./Test_files/spellCounts/afterBanana.json");

        int playThroughDepth{12};
        int nodeDepth{1};
        int dirtsForBanana{100};
        int clearSpaceForHeuristic{-1}; //if everything is clear for this distance, use heuristic
        bool patternDetectEnable{false};
        uint64_t mcTime_ns{880000000};
        float mc_c{std::sqrt(2)};
        int mc_runsBeforeClockCheck{50};

        GetEvaluatorFn_t eval = [&](bool, GameStatePtr){ return Evaluators::RushHealth; };

        Bot bot(eval,
                playThroughDepth, nodeDepth, 
                dirtsForBanana, clearSpaceForHeuristic, patternDetectEnable, NextTurn::WormCanShoot,
                mcTime_ns, mc_c, mc_runsBeforeClockCheck);

        WHEN("We request the next move")
        {
            auto ret = bot.runStrategy(roundJSON);

            THEN("Nothing breaks")
            {
                REQUIRE(true);
            }

            AND_THEN("We run another move")
            {
                auto ret = bot.runStrategy(round2JSON);
                THEN("Nothing breaks")
                {
                    REQUIRE(true);
                }   
            }
        }
    }
}