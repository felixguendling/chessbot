#include "doctest/doctest.h"

#include <iomanip>
#include <iostream>

#include "chessbot/generate_moves.h"
#include "chessbot/nn.h"
#include "chessbot/position.h"

using namespace chessbot;

TEST_CASE("nn classifies legal moves") {
  auto const fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  auto p = position::from_fen(fen);
  auto moves = std::array<move, max_moves>{};
  auto moves_end = generate_moves(p, moves);

  network<128, 1024, 1024, 1024, 1> n{0.0, 100.0};

  for (auto i = 0; i < 100000; ++i) {
    auto const from = get_random_number() % 64;
    auto const to = get_random_number() % 64;
    for (auto j = 0; j < 50; ++j) {
      auto in = std::array<real_t, 128>{};
      in[from] = 1;
      in[64 + to] = 1;
      n.train(in,
              std::array{std::any_of(&moves[0], moves_end,
                                     [&](move const m) {
                                       return m.from_field_ == from &&
                                              m.to_field_ == to;
                                     })},
              0.4);
    }
  }
}