#include "doctest/doctest.h"

#include <iostream>
#include <sstream>

#include "chessbot/constants.h"
#include "chessbot/position.h"

using namespace chessbot;

TEST_CASE("check repetition en passant") {
  auto in = std::stringstream{start_position_fen};

  auto p = chessbot::position{};
  in >> p;

  p.make_move("e2e3");  // half move count = 0
  p.make_move("e7e5");  // half move count = 0, en passant square = e6

  auto repeat = [&p, i = 0]() mutable {
    p.make_move("g1f3");
    CHECK(p.new_repetitions() == i);

    p.make_move("b8c6");
    CHECK(p.new_repetitions() == i);

    p.make_move("f3g1");
    CHECK(p.new_repetitions() == i);

    p.make_move("c6b8");
    CHECK(p.new_repetitions() == i);

    ++i;
  };

  repeat();
  repeat();
  repeat();
  repeat();
}

TEST_CASE("check repetition castling") {
  auto in = std::stringstream{start_position_fen};

  auto p = chessbot::position{};
  in >> p;
  p.make_move("g1f3");
  CHECK(p.new_repetitions() == 0);

  p.make_move("b8c6");
  CHECK(p.new_repetitions() == 0);

  p.make_move("f3g1");
  CHECK(p.new_repetitions() == 0);

  p.make_move("c6b8");
  CHECK(p.new_repetitions() == 1);

  p.make_move("g1f3");
  CHECK(p.new_repetitions() == 1);

  p.make_move("b8c6");
  CHECK(p.new_repetitions() == 1);

  p.make_move("h1g1");
  CHECK(p.new_repetitions() == 0);

  p.make_move("a8b8");
  CHECK(p.new_repetitions() == 0);

  p.make_move("g1h1");
  CHECK(p.new_repetitions() == 0);

  p.make_move("b8a8");
  CHECK(p.new_repetitions() == 0);

  p.make_move("h1g1");
  CHECK(p.new_repetitions() == 0);

  p.make_move("a8b8");
  CHECK(p.new_repetitions() == 1);

  p.make_move("g1h1");
  CHECK(p.new_repetitions() == 1);

  p.make_move("b8a8");
  CHECK(p.new_repetitions() == 1);
}

TEST_CASE("check repetition undo") {
  auto in = std::stringstream{start_position_fen};

  auto p = chessbot::position{};
  in >> p;

  auto const s = p.make_move("e2e3");
  p.undo_move(s);
  std::cout << "HALF MOVE CLOCK AFTER UNDO: " << (int)p.half_move_clock_
            << "\n";
  std::cout << "HASH AFTER UNDO: " << p.hashes_[p.half_move_clock_] << "\n";
  std::cout << "FEN AFTER UNDO: " << p.to_fen() << "\n";

  auto repeat = [&p, i = 0]() mutable {
    p.make_move("g1f3");
    CHECK(p.new_repetitions() == i);

    p.make_move("b8c6");
    CHECK(p.new_repetitions() == i);

    p.make_move("f3g1");
    CHECK(p.new_repetitions() == i);

    p.make_move("c6b8");
    CHECK(p.new_repetitions() == ++i);
  };

  repeat();
  repeat();
  repeat();
  repeat();
}