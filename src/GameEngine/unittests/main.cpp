#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include "NextTurn.hpp"
#include "GameState.hpp"

struct MyListener : Catch::TestEventListenerBase {

    using TestEventListenerBase::TestEventListenerBase; // inherit constructor

    void testCaseStarting( Catch::TestCaseInfo const& testInfo ) override {
        GameState::Initialise();
    }
    
    void testCaseEnded( Catch::TestCaseStats const& testCaseStats ) override {
        // Tear-down after a test case is run
    }
};
CATCH_REGISTER_LISTENER( MyListener )

int main( int argc, char* argv[] ) {
  // global setup...
  NextTurn::Initialise();

  int result = Catch::Session().run( argc, argv );

  // global clean-up...

  return result;
}