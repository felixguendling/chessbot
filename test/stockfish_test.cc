#include <iostream>

#include "doctest/doctest.h"

#include "chessbot/stockfish_evals.h"

using namespace chessbot;

TEST_CASE("get stockfish evals 1") {
  auto p = position::from_fen("K7/5r2/k7/8/8/8/8/8 b - - 0 1");
  auto const evals = stockfish_evals(p);
  CHECK(evals.at("f7f8").mate_ == 1);
  CHECK(evals.at("a6b6").mate_ == 2);
  CHECK(evals.at("f7b7").mate_ == 0);
  CHECK(evals.at("f7b7").cp_ == 0);
}

TEST_CASE("get stockfish evals 2") {
  auto p = position::from_fen("K1N5/5r2/k7/8/8/8/8/8 b - - 0 1");
  auto const evals = stockfish_evals(p);
  CHECK(evals.at("f7e7").mate_ == 0);
  CHECK(evals.at("f7e7").cp_ == -10);
}