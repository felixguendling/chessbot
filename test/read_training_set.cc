#include <sstream>

#include "chessbot/read_training_set.h"
#include "doctest/doctest.h"

using namespace chessbot;

TEST_CASE("read training set") {
  auto s =
      R"(r3qk1r/2Q2ppp/1p6/4Pb2/4pP2/8/PPP3PP/R2R2K1 b - - 0 20 a8a2 -797 a8a3 -732 a8a4 -571 a8a5 -563 a8a6 -584 a8a7 -809 a8b8 249 a8c8 152 a8d8 -1148 b6b5 320 e4e3 178 e8a4 -317 e8b5 -283 e8b8 294 e8c6 -1151 e8c8 232 e8d7 -755 e8d8 M-2 e8e5 -1063 e8e6 -306 e8e7 198 f5c8 -561 f5d7 -644 f5e6 217 f5g4 195 f5g6 0 f5h3 -352 f7f6 115 f8g8 240 g7g5 152 g7g6 237 h7h5 222 h7h6 215 h8g8 188
rnbqkbnr/pppppp1p/6p1/8/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 2 a2a3 5 a2a4 9 b1a3 -5 b1c3 77 b1d2 30 b2b3 -13 b2b4 16 c1d2 6 c1e3 -4 c1f4 33 c1g5 12 c1h6 -567 c2c3 11 c2c4 58 d1d2 4 d1d3 0 d4d5 34 e1d2 -88 e2e3 25 e2e4 75 f2f3 16 f2f4 -35 g1f3 35 g1h3 -6 g2g3 23 g2g4 -50 h2h3 15 h2h4 44
8/6R1/2R5/1P6/7k/P4P2/6p1/6K1 w - - 5 52 a3a4 M3 b5b6 M3 c6a6 M3 c6b6 M3 c6c1 M3 c6c2 M3 c6c3 M3 c6c4 M3 c6c5 M2 c6c7 M3 c6c8 M2 c6d6 M3 c6e6 M3 c6f6 M3 c6g6 M3 c6h6 M1 f3f4 M3 g1f2 M4 g1g2 M3 g1h2 M4 g7a7 M5 g7b7 M5 g7c7 M5 g7d7 M5 g7e7 M5 g7f7 M5 g7g2 M3 g7g3 6777 g7g4 M3 g7g5 6777 g7g6 M3 g7g8 M3 g7h7 M5)";
  auto ss = std::stringstream{};
  ss << s;
  auto const positions = read_training_set(ss);
  REQUIRE(positions.size() == 3);
  CHECK("r3qk1r/2Q2ppp/1p6/4Pb2/4pP2/8/PPP3PP/R2R2K1 b - - 0 20" ==
        positions[0].first.to_fen());
  CHECK("rnbqkbnr/pppppp1p/6p1/8/3P4/8/PPP1PPPP/RNBQKBNR w KQkq - 0 2" ==
        positions[1].first.to_fen());
  CHECK(positions[0].second.at("a8a7").cp_ == -809);
  CHECK(positions[1].second.at("c1d2").cp_ == 6);
  CHECK(positions[2].second.at("b5b6").cp_ == 0);
  CHECK(positions[2].second.at("b5b6").mate_ == 3);
}
