#include "doctest/doctest.h"

#include <sstream>

#include "chessbot/constants.h"
#include "chessbot/position.h"

using namespace chessbot;

TEST_CASE("print start position") {
  auto const start_position =
      std::string{R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 | bR | bN | bB | bQ | bK | bB | bN | bR |
7 | bP | bP | bP | bP | bP | bP | bP | bP |
6 |    |    |    |    |    |    |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 |    |    |    |    |    |    |    |    |
2 | wP | wP | wP | wP | wP | wP | wP | wP |
1 | wR | wN | wB | wQ | wK | wB | wN | wR |
)"};

  auto in = std::stringstream{start_position_fen};

  auto p = chessbot::position{};
  in >> p;

  CHECK(p.to_fen() == std::string{start_position_fen});
  CHECK(p.to_move_ == chessbot::color::WHITE);
  CHECK(p.castling_rights_.white_can_short_castle_ == true);
  CHECK(p.castling_rights_.white_can_long_castle_ == true);
  CHECK(p.castling_rights_.black_can_short_castle_ == true);
  CHECK(p.castling_rights_.black_can_long_castle_ == true);
  CHECK(p.en_passant_ == 0U);
  CHECK(p.half_move_clock_ == 0U);
  CHECK(p.full_move_count_ == 1U);

  auto ss = std::stringstream{};
  ss << p;
  CHECK(ss.str() == start_position);
}

TEST_CASE("print two kings position") {
  auto const start_position =
      std::string{R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 |    |    |    |    |    |    |    |    |
5 |    |    | bK |    |    |    |    |    |
4 |    |    |    |    | wK |    |    |    |
3 |    |    |    |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)"};

  constexpr auto const position_fen = "8/8/8/2k5/4K3/8/8/8 w Qq - 5 33";
  auto in = std::stringstream{position_fen};

  auto p = chessbot::position{};
  in >> p;

  CHECK(p.to_fen() == std::string{position_fen});
  CHECK(p.to_move_ == chessbot::color::WHITE);
  CHECK(p.castling_rights_.white_can_short_castle_ == false);
  CHECK(p.castling_rights_.white_can_long_castle_ == true);
  CHECK(p.castling_rights_.black_can_short_castle_ == false);
  CHECK(p.castling_rights_.black_can_long_castle_ == true);
  CHECK(p.en_passant_ == 0U);
  CHECK(p.half_move_clock_ == 5U);
  CHECK(p.full_move_count_ == 33U);

  auto ss = std::stringstream{};
  ss << p;
  CHECK(ss.str() == start_position);
}

TEST_CASE("print random position") {
  auto const position =
      std::string{R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 |    |    |    |    |    |    |    |    |
5 | bP | wP | bK |    |    |    |    |    |
4 |    |    |    |    | wK |    |    |    |
3 |    |    |    |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)"};

  constexpr auto const position_fen = "8/8/8/pPk5/4K3/8/8/8 w - a6 5 33";
  auto in = std::stringstream{position_fen};

  auto p = chessbot::position{};
  in >> p;

  CHECK(p.to_fen() == std::string{position_fen});
  CHECK(p.to_move_ == chessbot::color::WHITE);
  CHECK(p.castling_rights_.white_can_short_castle_ == false);
  CHECK(p.castling_rights_.white_can_long_castle_ == false);
  CHECK(p.castling_rights_.black_can_short_castle_ == false);
  CHECK(p.castling_rights_.black_can_long_castle_ == false);
  CHECK(p.en_passant_ == chessbot::rank_file_to_bitboard(2, 0));
  CHECK(p.half_move_clock_ == 5U);
  CHECK(p.full_move_count_ == 33U);

  auto ss = std::stringstream{};
  ss << p;
  CHECK(ss.str() == position);
}

TEST_CASE("fen output") {
  auto in = std::stringstream{start_position_fen};

  auto p = chessbot::position{};
  in >> p;

  CHECK(p.to_fen() == std::string{start_position_fen});
}