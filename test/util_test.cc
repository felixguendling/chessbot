#include "doctest/doctest.h"

#include <iostream>
#include <sstream>

#include "chessbot/constants.h"
#include "chessbot/position.h"
#include "chessbot/util.h"

using namespace chessbot;

TEST_CASE("square name test") { CHECK(get_square_name(1U) == "a8"); }

TEST_CASE("white short castle knight attacks") {
  position p;
  p.piece_states_[PAWN] = short_castle_knight_attack_squares[color::WHITE];
  p.pieces_by_color_[color::WHITE] =
      short_castle_knight_attack_squares[color::WHITE];
  std::stringstream ss;
  ss << p;
  CHECK(ss.str() == R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 |    |    |    |    |    |    |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 |    |    |    | wP | wP | wP | wP | wP |
2 |    |    | wP | wP | wP |    | wP | wP |
1 |    |    |    |    |    |    |    |    |
)");
}

TEST_CASE("black short castle knight attacks") {
  position p;
  p.piece_states_[PAWN] = short_castle_knight_attack_squares[color::BLACK];
  p.pieces_by_color_[color::WHITE] =
      short_castle_knight_attack_squares[color::BLACK];
  std::stringstream ss;
  ss << p;
  CHECK(ss.str() == R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 |    |    | wP | wP | wP |    | wP | wP |
6 |    |    |    | wP | wP | wP | wP | wP |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 |    |    |    |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)");
}

TEST_CASE("white long castle knight attacks") {
  position p;
  p.piece_states_[PAWN] = long_castle_knight_attack_squares[color::WHITE];
  p.pieces_by_color_[color::WHITE] =
      long_castle_knight_attack_squares[color::WHITE];
  std::stringstream ss;
  ss << p;
  CHECK(ss.str() == R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 |    |    |    |    |    |    |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 |    | wP | wP | wP | wP | wP |    |    |
2 | wP | wP | wP |    | wP | wP | wP |    |
1 |    |    |    |    |    |    |    |    |
)");
}

TEST_CASE("black long castle knight attacks") {
  position p;
  p.piece_states_[PAWN] = long_castle_knight_attack_squares[color::BLACK];
  p.pieces_by_color_[color::WHITE] =
      long_castle_knight_attack_squares[color::BLACK];
  std::stringstream ss;
  ss << p;
  CHECK(ss.str() == R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 | wP | wP | wP |    | wP | wP | wP |    |
6 |    | wP | wP | wP | wP | wP |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 |    |    |    |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)");
}
