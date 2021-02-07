#include "doctest/doctest.h"

#include <iostream>
#include <sstream>

#include "chessbot/move.h"
#include "chessbot/position.h"

TEST_CASE("pawn moves") {
  auto in = std::stringstream{"8/8/8/8/8/p7/1P6/8 w - - 0 1"};

  constexpr auto const move_board =
      R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 |    |    |    |    |    |    |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 | bP | wP |    |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)";

  constexpr auto const double_jump_board =
      R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 |    |    |    |    |    |    |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    | wP |    |    |    |    |    |    |
3 | bP |    |    |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)";

  constexpr auto const capture_board =
      R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 |    |    |    |    |    |    |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 | wP |    |    |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)";

  chessbot::position p;
  in >> p;

  auto i = 0U;
  chessbot::for_each_possible_move(p, [&](chessbot::move const& m) {
    auto const board = p.make_move(m).to_str();
    auto const valid = board == move_board || board == capture_board ||
                       board == double_jump_board;
    ++i;
    CHECK(valid);
  });
  CHECK(i == 3);
}