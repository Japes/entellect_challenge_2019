#define CATCH_CONFIG_RUNNER
#include "catch.hpp"
#include "NextTurn.hpp"

int main( int argc, char* argv[] ) {
  // global setup...
  NextTurn::Initialise();

  int result = Catch::Session().run( argc, argv );

  // global clean-up...

  return result;
}