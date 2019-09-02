#include "catch.hpp"
#include "../GameEngine/PatternDetector.hpp"
#include "../GameEngine/Commands/AllCommands.hpp"

TEST_CASE( "Prediction, nice tests", "[patterndetector]" ) {
    GIVEN("A pattern detector of length 1")
    {
        PatternDetector pat(1);

        WHEN("We pass it nothing")
        {
            THEN("It returns null")
            {
                auto ret = pat.Prediction();
                REQUIRE(ret == nullptr);
            }
        }

        WHEN("We pass it a pattern of length 1")
        {
            pat.AddCommand(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::N));
            pat.AddCommand(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::N));
            pat.AddCommand(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::N));

            THEN("It can make predictions")
            {
                auto ret = pat.Prediction();
                REQUIRE(ret != nullptr);
                REQUIRE(ret->GetCommandString() == "shoot N");
            }
        }

        WHEN("We pass it a pattern of length 2")
        {
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,1)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,1)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));

            THEN("It cant make predictions")
            {
                auto ret = pat.Prediction();
                REQUIRE(ret == nullptr);
            }
        }
    }

    GIVEN("A pattern detector of length 2")
    {
        PatternDetector pat(2);
        WHEN("We pass it a pattern of length 2")
        {
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,1)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,1)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));

            THEN("It can make predictions")
            {
                auto ret = pat.Prediction();
                REQUIRE(ret != nullptr);
                REQUIRE(ret->GetCommandString() == "move 1 1");
            }
        }

        WHEN("We pass it a pattern of length 3")
        {
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,1)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,1)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));

            THEN("It cant make predictions")
            {
                auto ret = pat.Prediction();
                REQUIRE(ret == nullptr);
            }
        }
    }

    GIVEN("A pattern detector of length 3")
    {
        PatternDetector pat(3);
        WHEN("We pass it a pattern of length 3")
        {
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,1)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,1)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));

            THEN("It can make predictions")
            {
                auto ret = pat.Prediction();
                REQUIRE(ret != nullptr);
                REQUIRE(ret->GetCommandString() == "move 1 1");
            }
        }
    }
}

TEST_CASE( "Prediction, weird tests", "[patterndetectorweird]" ) {

    GIVEN("A pattern detector of length 3")
    {
        PatternDetector pat(3);
        WHEN("We pass it a pattern of length 1")
        {
            pat.AddCommand(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::N));
            pat.AddCommand(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::N));
            pat.AddCommand(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::N));
            pat.AddCommand(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::N));
            pat.AddCommand(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::N));

            THEN("It can make predictions")
            {
                auto ret = pat.Prediction();
                REQUIRE(ret != nullptr);
                REQUIRE(ret->GetCommandString() == "shoot N");
            }
        }

        WHEN("We pass it a pattern of length 2")
        {
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,1)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,1)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));

            THEN("It can make predictions")
            {
                auto ret = pat.Prediction();
                REQUIRE(ret != nullptr);
                REQUIRE(ret->GetCommandString() == "move 1 1");
            }
        }
    }

    GIVEN("A pattern detector of length 2")
    {
        PatternDetector pat(2);
        WHEN("We pass it a pattern of length 1")
        {
            pat.AddCommand(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::N));
            pat.AddCommand(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::N));
            pat.AddCommand(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::N));
            pat.AddCommand(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::N));
            pat.AddCommand(std::make_shared<ShootCommand>(ShootCommand::ShootDirection::N));

            THEN("It can make predictions")
            {
                auto ret = pat.Prediction();
                REQUIRE(ret != nullptr);
                REQUIRE(ret->GetCommandString() == "shoot N");
            }
        }

        WHEN("We break the pattern on the last update")
        {
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,1)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,1)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(2,1)));

            THEN("It returns null")
            {
                auto ret = pat.Prediction();
                REQUIRE(ret == nullptr);
            }
        }

        WHEN("The inner pattern is different")
        {
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,1)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));

            THEN("It returns null")
            {
                auto ret = pat.Prediction();
                REQUIRE(ret == nullptr);
            }
        }

        WHEN("markers are different")
        {
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,1)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,1)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));

            THEN("It returns null")
            {
                auto ret = pat.Prediction();
                REQUIRE(ret == nullptr);
            }
        }

        WHEN("We don't pass it enough info to pick up a pattern")
        {
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,1)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(0,0)));
            pat.AddCommand(std::make_shared<TeleportCommand>(Position(1,1)));

            THEN("It returns null")
            {
                auto ret = pat.Prediction();
                REQUIRE(ret == nullptr);
            }
        }
    }


}