#include "doctest/doctest.h"

#include <iostream>

#include "chessbot/mcts.h"

using namespace chessbot;

TEST_CASE("mcts mate in 1") {
  auto p = position::from_fen("K3n3/3n4/k7/8/8/8/8/8 b - - 0 1");

  auto next_moves = std::array<move, max_moves>();
  auto next_end_evaluations = std::array<float, max_moves>();
  mcts<true, false>(p, 1000, nullptr, next_moves, next_end_evaluations);

  std::cout << "move evaluation:\n";
  for (auto i = 0U; i < max_moves; ++i) {
    if (!next_moves[i].is_initialized()) {
      break;
    }
    std::cout << " -> " << next_moves[i].to_str() << ": "
              << next_end_evaluations[i] << "\n";
  }

  auto const best_move_it =
      std::max_element(begin(next_end_evaluations), end(next_end_evaluations));
  std::cout
      << "best move: "
      << next_moves[std::distance(begin(next_end_evaluations), best_move_it)]
      << "\n";
}

TEST_CASE("mcts mate in 2") {
  srand(1);
  auto p = position::from_fen("8/2K5/6r1/7r/8/8/8/2k5 b - - 0 1");

  auto next_moves = std::array<move, max_moves>();
  auto next_end_evaluations = std::array<float, max_moves>();
  mcts<true, true>(p, 1000, nullptr, next_moves, next_end_evaluations);

  std::cout << "move evaluation:\n";
  for (auto i = 0U; i < max_moves; ++i) {
    if (!next_moves[i].is_initialized()) {
      break;
    }
    std::cout << " -> " << next_moves[i].to_str() << ": "
              << next_end_evaluations[i] << "\n";
  }

  auto const best_move_it =
      std::max_element(begin(next_end_evaluations), end(next_end_evaluations));
  std::cout
      << "best move: "
      << next_moves[std::distance(begin(next_end_evaluations), best_move_it)]
      << "\n";
}