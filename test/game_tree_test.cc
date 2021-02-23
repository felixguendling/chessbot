#include <atomic>
#include <sstream>
#include <vector>

#include "doctest/doctest.h"

#include "utl/parallel_for.h"

#include "chessbot/generate_moves.h"
#include "chessbot/position.h"
#include "chessbot/timing.h"

using namespace chessbot;

TEST_CASE("compute all moves") {
  auto in = std::stringstream{start_position_fen};

  auto p = chessbot::position{};
  in >> p;

  auto const start_state_info = p.to_state_info();

  //  p.make_move("g1f3");
  //  p.make_move("a2a3");
  //  p.make_move("c7c6");
  //  p.make_move("b2b3");
  //  p.make_move("d8b6");
  //  p.make_move("e2e3");
  //  p.make_move("b6e3");
}