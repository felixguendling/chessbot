#include "doctest/doctest.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "chessbot/constants.h"
#include "chessbot/generate_moves.h"
#include "chessbot/position.h"

using namespace chessbot;

struct test_position : position {
  explicit test_position(std::string const& fen)
      : position{position::from_fen(start_position_fen)} {}

  void make_move(std::string const& s) {
    auto const& m =
        states_.emplace_back(std::make_unique<state_info>(position::make_move(
            s, states_.empty() ? nullptr : states_.back().get())));
  }

  unsigned count_repetitions() const {
    return chessbot::count_repetitions(*this, states_.back().get(), get_hash());
  }

  void print_trace() const { position::print_trace(states_.back().get()); }

  std::vector<std::unique_ptr<state_info>> states_;
};

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

TEST_CASE("check repetition undo") {
  auto p = test_position{start_position_fen};

  p.make_move("e2e3");
  p.undo_move(*p.states_.back());
  p.states_.resize(p.states_.size() - 1U);

  auto repeat = [&p, i = 0]() mutable {
    p.make_move("g1f3");
    CHECK(p.count_repetitions() == i);

    p.make_move("b8c6");
    CHECK(p.count_repetitions() == i);

    p.make_move("f3g1");
    CHECK(p.count_repetitions() == i);

    p.make_move("c6b8");
    CHECK(p.count_repetitions() == ++i);
  };

  repeat();
  repeat();
  repeat();
  repeat();
}
