#include "doctest/doctest.h"

#include <iostream>
#include <set>
#include <sstream>
#include <string>

#include "chessbot/move.h"
#include "chessbot/position.h"

std::set<std::string> print_all_positions_after_move(
    chessbot::position const& p) {
  std::set<std::string> prints;
  chessbot::for_each_possible_move(p, [&](chessbot::move const& m) {
    prints.emplace(p.make_move(m).to_str());
  });
  return prints;
};

std::set<std::string> fen_strings_after_move(chessbot::position const& p) {
  std::set<std::string> prints;
  chessbot::for_each_possible_move(p, [&](chessbot::move const& m) {
    prints.emplace(p.make_move(m).to_fen());
  });
  return prints;
};

TEST_CASE("white pawn moves") {
  auto in = std::stringstream{"8/8/8/8/8/p1p5/1P6/8 w - - 0 1"};

  constexpr auto const move_board =
      R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 |    |    |    |    |    |    |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 | bP | wP | bP |    |    |    |    |    |
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
3 | bP |    | bP |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)";

  constexpr auto const capture_board_1 =
      R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 |    |    |    |    |    |    |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 | wP |    | bP |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)";

  constexpr auto const capture_board_2 =
      R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 |    |    |    |    |    |    |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 | bP |    | wP |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)";

  chessbot::position p;
  in >> p;

  CHECK(print_all_positions_after_move(p) ==
        std::set<std::string>{move_board, double_jump_board, capture_board_1,
                              capture_board_2});
}

TEST_CASE("black pawn moves") {
  constexpr auto const move_board =
      R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 | wP | bP | wP |    |    |    |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 |    |    |    |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)";

  constexpr auto const double_jump_board =
      R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 | wP |    | wP |    |    |    |    |    |
5 |    | bP |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 |    |    |    |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)";

  constexpr auto const capture_board_1 =
      R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 | bP |    | wP |    |    |    |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 |    |    |    |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)";

  constexpr auto const capture_board_2 =
      R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 | wP |    | bP |    |    |    |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 |    |    |    |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)";

  auto in = std::stringstream{"8/1p6/P1P5/8/8/8/8/8 b - - 0 1"};
  chessbot::position p;
  in >> p;
  CHECK(print_all_positions_after_move(p) ==
        std::set<std::string>{move_board, double_jump_board, capture_board_1,
                              capture_board_2});
}

TEST_CASE("en passant white") {
  constexpr auto const capture_en_passant =
      R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 | wP | bP |    |    |    |    |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 |    |    |    |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)";

  auto in = std::stringstream{"8/8/1p6/pP6/8/8/8/8 w - a6 0 1"};
  chessbot::position p;
  in >> p;
  CHECK(print_all_positions_after_move(p) ==
        std::set<std::string>{capture_en_passant});
}

TEST_CASE("en passant black") {
  constexpr auto const capture_en_passant =
      R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 |    |    |    |    |    |    |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 | wK | bP |    |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)";

  auto in = std::stringstream{"8/8/8/8/pP6/K7/8/8 b - b3 0 1"};
  chessbot::position p;
  in >> p;
  CHECK(print_all_positions_after_move(p) ==
        std::set<std::string>{capture_en_passant});
}

TEST_CASE("en passant white") {
  auto in = std::stringstream{"8/4p3/8/8/8/8/8/8 b - - 0 1"};
  chessbot::position p;
  in >> p;
  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{"8/8/4p3/8/8/8/8/8 w - - 0 2",
                              "8/8/8/4p3/8/8/8/8 w - e6 0 2"});
}

TEST_CASE("white pawn moves blocked") {
  auto in = std::stringstream{"8/8/8/8/6p1/6Pp/p6P/8 w - - 0 1"};
  chessbot::position p;
  in >> p;
  CHECK(print_all_positions_after_move(p).empty());
}

TEST_CASE("black pawn moves blocked") {
  auto in = std::stringstream{"8/p7/P7/8/8/8/8/8 b - - 0 1"};
  chessbot::position p;
  in >> p;
  CHECK(print_all_positions_after_move(p).empty());
}

TEST_CASE("white pawn promotion") {
  auto in = std::stringstream{"5q2/4P3/8/8/8/8/8/8 w - - 0 1"};
  chessbot::position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{
            "5N2/8/8/8/8/8/8/8 b - - 0 1", "5B2/8/8/8/8/8/8/8 b - - 0 1",
            "5R2/8/8/8/8/8/8/8 b - - 0 1", "5Q2/8/8/8/8/8/8/8 b - - 0 1",
            "4Nq2/8/8/8/8/8/8/8 b - - 0 1", "4Bq2/8/8/8/8/8/8/8 b - - 0 1",
            "4Rq2/8/8/8/8/8/8/8 b - - 0 1", "4Qq2/8/8/8/8/8/8/8 b - - 0 1"});
}

TEST_CASE("white pawn promotion") {
  auto in = std::stringstream{"8/8/8/8/8/8/4p3/5Q2 b - - 0 1"};
  chessbot::position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{
            "8/8/8/8/8/8/8/5n2 w - - 0 2", "8/8/8/8/8/8/8/5b2 w - - 0 2",
            "8/8/8/8/8/8/8/5r2 w - - 0 2", "8/8/8/8/8/8/8/5q2 w - - 0 2",
            "8/8/8/8/8/8/8/4nQ2 w - - 0 2", "8/8/8/8/8/8/8/4bQ2 w - - 0 2",
            "8/8/8/8/8/8/8/4rQ2 w - - 0 2", "8/8/8/8/8/8/8/4qQ2 w - - 0 2"});
}

TEST_CASE("knight move") {
  auto in = std::stringstream{"N6N/8/8/8/8/8/8/N6N w - - 0 1"};
  chessbot::position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{
            "7N/2N5/8/8/8/8/8/N6N b - - 0 1", "7N/8/1N6/8/8/8/8/N6N b - - 0 1",
            "N7/5N2/8/8/8/8/8/N6N b - - 0 1", "N7/8/6N1/8/8/8/8/N6N b - - 0 1",
            "N6N/8/8/8/8/8/2N5/7N b - - 0 1", "N6N/8/8/8/8/1N6/8/7N b - - 0 1",
            "N6N/8/8/8/8/8/5N2/N7 b - - 0 1",
            "N6N/8/8/8/8/6N1/8/N7 b - - 0 1"});
}

TEST_CASE("knight move blocked") {
  auto in = std::stringstream{"N1q4N/2P5/8/8/8/8/8/N6N w - - 0 1"};
  chessbot::position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{"2q4N/2P5/1N6/8/8/8/8/N6N b - - 0 1",
                              "N1q5/2P2N2/8/8/8/8/8/N6N b - - 0 1",
                              "N1q5/2P5/6N1/8/8/8/8/N6N b - - 0 1",
                              "N1q4N/2P5/8/8/8/8/2N5/7N b - - 0 1",
                              "N1q4N/2P5/8/8/8/1N6/8/7N b - - 0 1",
                              "N1q4N/2P5/8/8/8/8/5N2/N7 b - - 0 1",
                              "N1q4N/2P5/8/8/8/6N1/8/N7 b - - 0 1"});
}

TEST_CASE("knight move capture") {
  auto in = std::stringstream{"N7/2p5/8/8/8/8/8/8 w - - 0 1"};
  chessbot::position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{"8/2p5/1N6/8/8/8/8/8 b - - 0 1",
                              "8/2N5/8/8/8/8/8/8 b - - 0 1"});
}