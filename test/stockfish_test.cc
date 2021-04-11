#include <iostream>

#include "doctest/doctest.h"

#include "chessbot/stockfish_evals.h"

using namespace chessbot;

TEST_CASE("get stockfish evals") {
  auto p = position::from_fen(start_position_fen);
  auto const evals = stockfish_evals(p);

  for (auto const& [k, v] : evals) {
    std::cout << "move: " << k << " eval: " << v << "\n";
  }
}
