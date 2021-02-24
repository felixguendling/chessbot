#include "doctest/doctest.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "chessbot/constants.h"
#include "chessbot/generate_moves.h"
#include "chessbot/position.h"

#include "./test_position.h"

using namespace chessbot;

TEST_CASE("check repetition en passant") {
  auto p = test_position{start_position_fen};
  p.make_move("e2e3");
  p.make_move("e7e5");

  auto repeat = [&p, i = 0]() mutable {
    p.make_move("g1f3");
    CHECK(p.count_repetitions() == i);

    p.make_move("b8c6");
    CHECK(p.count_repetitions() == i);

    p.make_move("f3g1");
    CHECK(p.count_repetitions() == i);

    p.make_move("c6b8");
    CHECK(p.count_repetitions() == i);

    ++i;
  };

  repeat();
  repeat();
  repeat();
  repeat();
}

TEST_CASE("check repetition castling") {
  auto p = test_position{start_position_fen};

  p.make_move("g1f3");
  CHECK(p.count_repetitions() == 0);

  p.make_move("b8c6");
  CHECK(p.count_repetitions() == 0);

  p.make_move("f3g1");
  CHECK(p.count_repetitions() == 0);

  p.make_move("c6b8");
  CHECK(p.count_repetitions() == 1);

  p.make_move("g1f3");
  CHECK(p.count_repetitions() == 1);

  p.make_move("b8c6");
  CHECK(p.count_repetitions() == 1);

  p.make_move("h1g1");
  CHECK(p.count_repetitions() == 0);

  p.make_move("a8b8");
  CHECK(p.count_repetitions() == 0);

  p.make_move("g1h1");
  CHECK(p.count_repetitions() == 0);

  p.make_move("b8a8");
  CHECK(p.count_repetitions() == 0);

  p.make_move("h1g1");
  CHECK(p.count_repetitions() == 0);

  p.make_move("a8b8");
  CHECK(p.count_repetitions() == 1);

  p.make_move("g1h1");
  CHECK(p.count_repetitions() == 1);

  p.make_move("b8a8");
  CHECK(p.count_repetitions() == 1);
}
