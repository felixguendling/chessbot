#include "doctest/doctest.h"

#include <iostream>
#include <set>
#include <sstream>
#include <string>

#include "utl/pipes.h"

#include "chessbot/dfs.h"
#include "chessbot/generate_moves.h"
#include "chessbot/position.h"

#include "./test_position.h"

using namespace chessbot;

std::set<std::string> print_all_positions_after_move(position const& p) {
  p.validate();
  std::set<std::string> prints;
  auto move_list = std::array<move, max_moves>{};
  auto const end = generate_moves(p, &move_list[0]);
  for (auto const& m : utl::all(&move_list[0], end) | utl::iterable()) {
    auto copy = position{p};
    copy.make_move(m, nullptr);
    copy.validate();
    prints.emplace(copy.to_str());
  }
  return prints;
};

std::set<std::string> fen_strings_after_move(position const& p) {
  p.validate();
  std::set<std::string> prints;
  auto move_list = std::array<move, max_moves>{};
  auto const end = generate_moves(p, &move_list[0]);
  for (auto const& m : utl::all(&move_list[0], end) | utl::iterable()) {
    auto copy = position{p};
    copy.make_move(m, nullptr);
    copy.validate();
    prints.emplace(copy.to_fen());
  }
  return prints;
};

TEST_CASE("white pawn moves") {
  auto in = std::stringstream{"K1k5/8/2n5/8/8/p1p5/1P6/8 w - - 0 1"};

  constexpr auto const move_board =
      R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 | wK |    | bK |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 |    |    | bN |    |    |    |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 | bP | wP | bP |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)";

  constexpr auto const double_jump_board =
      R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 | wK |    | bK |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 |    |    | bN |    |    |    |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    | wP |    |    |    |    |    |    |
3 | bP |    | bP |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)";

  constexpr auto const capture_board_1 =
      R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 | wK |    | bK |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 |    |    | bN |    |    |    |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 | wP |    | bP |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)";

  constexpr auto const capture_board_2 =
      R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 | wK |    | bK |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 |    |    | bN |    |    |    |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 | bP |    | wP |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)";

  position p;
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
3 |    |    |    |    |    | wN |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    | wK |    | bK |
)";

  constexpr auto const double_jump_board =
      R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 | wP |    | wP |    |    |    |    |    |
5 |    | bP |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 |    |    |    |    |    | wN |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    | wK |    | bK |
)";

  constexpr auto const capture_board_1 =
      R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 | bP |    | wP |    |    |    |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 |    |    |    |    |    | wN |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    | wK |    | bK |
)";

  constexpr auto const capture_board_2 =
      R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    |    |
7 |    |    |    |    |    |    |    |    |
6 | wP |    | bP |    |    |    |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 |    |    |    |    |    | wN |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    | wK |    | bK |
)";

  auto in = std::stringstream{"8/1p6/P1P5/8/8/5N2/8/5K1k b - - 0 1"};
  position p;
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
3 |    |    |    |    |    | bN |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    | bK |    | wK |
)";

  auto in = std::stringstream{"8/8/1p6/pP6/8/5n2/8/5k1K w - a6 0 1"};
  position p;
  in >> p;
  CHECK(print_all_positions_after_move(p) ==
        std::set<std::string>{capture_en_passant});
}

TEST_CASE("en passant black") {
  constexpr auto const capture_en_passant =
      R"(  | a  | b  | c  | d  | e  | f  | g  | h  |
8 |    |    |    |    |    |    |    | bK |
7 |    |    |    |    |    |    |    |    |
6 |    |    |    |    | wN | wN |    |    |
5 |    |    |    |    |    |    |    |    |
4 |    |    |    |    |    |    |    |    |
3 | wK | bP |    |    |    |    |    |    |
2 |    |    |    |    |    |    |    |    |
1 |    |    |    |    |    |    |    |    |
)";

  auto in = std::stringstream{"7k/8/4NN2/8/pP6/K7/8/8 b - b3 0 1"};
  position p;
  in >> p;
  CHECK(print_all_positions_after_move(p) ==
        std::set<std::string>{capture_en_passant});
}

TEST_CASE("en passant white") {
  auto in = std::stringstream{"8/4p3/8/8/8/2N5/8/k1K5 b - - 0 1"};
  position p;
  in >> p;
  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{"8/8/4p3/8/8/2N5/8/k1K5 w - - 0 2",
                              "8/8/8/4p3/8/2N5/8/k1K5 w - e6 0 2"});
}

TEST_CASE("white pawn moves blocked") {
  auto in = std::stringstream{"K1k5/8/2n5/8/6p1/6Pp/p6P/8 w - - 0 1"};
  position p;
  in >> p;
  CHECK(print_all_positions_after_move(p).empty());
}

TEST_CASE("black pawn moves blocked") {
  auto in = std::stringstream{"8/p7/P7/8/8/2N5/8/k1K5 b - - 0 1"};
  position p;
  in >> p;
  CHECK(print_all_positions_after_move(p).empty());
}

TEST_CASE("white pawn promotion") {
  auto in = std::stringstream{"5q2/4P3/8/8/8/2n5/8/K1k5 w - - 0 1"};
  position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{"5N2/8/8/8/8/2n5/8/K1k5 b - - 0 1",
                              "5B2/8/8/8/8/2n5/8/K1k5 b - - 0 1",
                              "5R2/8/8/8/8/2n5/8/K1k5 b - - 0 1",
                              "5Q2/8/8/8/8/2n5/8/K1k5 b - - 0 1",
                              "4Nq2/8/8/8/8/2n5/8/K1k5 b - - 0 1",
                              "4Bq2/8/8/8/8/2n5/8/K1k5 b - - 0 1",
                              "4Rq2/8/8/8/8/2n5/8/K1k5 b - - 0 1",
                              "4Qq2/8/8/8/8/2n5/8/K1k5 b - - 0 1"});
}

TEST_CASE("black pawn promotion") {
  auto in = std::stringstream{"k1K5/8/1P6/8/8/8/4p3/5Q2 b - - 0 1"};
  position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{"k1K5/8/1P6/8/8/8/8/5n2 w - - 0 2",
                              "k1K5/8/1P6/8/8/8/8/5b2 w - - 0 2",
                              "k1K5/8/1P6/8/8/8/8/5r2 w - - 0 2",
                              "k1K5/8/1P6/8/8/8/8/5q2 w - - 0 2",
                              "k1K5/8/1P6/8/8/8/8/4nQ2 w - - 0 2",
                              "k1K5/8/1P6/8/8/8/8/4bQ2 w - - 0 2",
                              "k1K5/8/1P6/8/8/8/8/4rQ2 w - - 0 2",
                              "k1K5/8/1P6/8/8/8/8/4qQ2 w - - 0 2"});
}

TEST_CASE("knight move") {
  auto in = std::stringstream{"N6N/8/8/8/8/8/8/N6N w - - 0 1"};
  position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{
            "7N/2N5/8/8/8/8/8/N6N b - - 1 1", "7N/8/1N6/8/8/8/8/N6N b - - 1 1",
            "N7/5N2/8/8/8/8/8/N6N b - - 1 1", "N7/8/6N1/8/8/8/8/N6N b - - 1 1",
            "N6N/8/8/8/8/8/2N5/7N b - - 1 1", "N6N/8/8/8/8/1N6/8/7N b - - 1 1",
            "N6N/8/8/8/8/8/5N2/N7 b - - 1 1",
            "N6N/8/8/8/8/6N1/8/N7 b - - 1 1"});
}

TEST_CASE("knight move blocked") {
  auto in = std::stringstream{"N1q4N/2P5/8/8/8/8/8/N6N w - - 0 1"};
  position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{"2q4N/2P5/1N6/8/8/8/8/N6N b - - 1 1",
                              "N1q5/2P2N2/8/8/8/8/8/N6N b - - 1 1",
                              "N1q5/2P5/6N1/8/8/8/8/N6N b - - 1 1",
                              "N1q4N/2P5/8/8/8/8/2N5/7N b - - 1 1",
                              "N1q4N/2P5/8/8/8/1N6/8/7N b - - 1 1",
                              "N1q4N/2P5/8/8/8/8/5N2/N7 b - - 1 1",
                              "N1q4N/2P5/8/8/8/6N1/8/N7 b - - 1 1"});
}

TEST_CASE("knight move capture") {
  auto in = std::stringstream{"N7/2p5/8/8/8/8/8/8 w - - 0 1"};
  position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{"8/2p5/1N6/8/8/8/8/8 b - - 1 1",
                              "8/2N5/8/8/8/8/8/8 b - - 0 1"});
}

TEST_CASE("bishop moves") {
  auto in = std::stringstream{"8/1p2p3/4P3/3B4/2p1p3/1p6/8/K1k5 w - - 0 1"};
  position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{"8/1p2p3/2B1P3/8/2p1p3/1p6/8/K1k5 b - - 1 1",
                              // captures
                              "8/1B2p3/4P3/8/2p1p3/1p6/8/K1k5 b - - 0 1",
                              "8/1p2p3/4P3/8/2B1p3/1p6/8/K1k5 b - - 0 1",
                              "8/1p2p3/4P3/8/2p1B3/1p6/8/K1k5 b - - 0 1"});
}

TEST_CASE("rook moves") {
  auto in = std::stringstream{"8/3p4/1p6/1P1R2p1/3p4/1p6/8/K1k5 w - - 0 1"};
  position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{"8/3p4/1p6/1PR3p1/3p4/1p6/8/K1k5 b - - 1 1",
                              "8/3p4/1p6/1P2R1p1/3p4/1p6/8/K1k5 b - - 1 1",
                              "8/3p4/1p6/1P3Rp1/3p4/1p6/8/K1k5 b - - 1 1",
                              "8/3p4/1p1R4/1P4p1/3p4/1p6/8/K1k5 b - - 1 1",
                              // captures
                              "8/3p4/1p6/1P4R1/3p4/1p6/8/K1k5 b - - 0 1",
                              "8/3p4/1p6/1P4p1/3R4/1p6/8/K1k5 b - - 0 1",
                              "8/3R4/1p6/1P4p1/3p4/1p6/8/K1k5 b - - 0 1"});
}

TEST_CASE("rook moves edge") {
  auto in = std::stringstream{"8/8/8/8/8/8/7p/5K1R w - - 0 1"};
  position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{
            // rook moves
            "8/8/8/8/8/8/7R/5K2 b - - 0 1", "8/8/8/8/8/8/7p/5KR1 b - - 1 1",

            // king moves
            "8/8/8/8/8/8/7p/4K2R b - - 1 1", "8/8/8/8/8/8/6Kp/7R b - - 1 1",
            "8/8/8/8/8/8/4K2p/7R b - - 1 1", "8/8/8/8/8/8/5K1p/7R b - - 1 1"});
}

TEST_CASE("queen moves") {
  auto in = std::stringstream{"8/8/2p1p3/2PQp3/2ppp3/1p6/8/K1k5 w - - 0 1"};
  position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{"8/8/2pQp3/2P1p3/2ppp3/1p6/8/K1k5 b - - 1 1",
                              "8/3Q4/2p1p3/2P1p3/2ppp3/1p6/8/K1k5 b - - 1 1",
                              "3Q4/8/2p1p3/2P1p3/2ppp3/1p6/8/K1k5 b - - 1 1",
                              // captures
                              "8/8/2Q1p3/2P1p3/2ppp3/1p6/8/K1k5 b - - 0 1",
                              "8/8/2p1p3/2P1Q3/2ppp3/1p6/8/K1k5 b - - 0 1",
                              "8/8/2p1Q3/2P1p3/2ppp3/1p6/8/K1k5 b - - 0 1",
                              "8/8/2p1p3/2P1p3/2Qpp3/1p6/8/K1k5 b - - 0 1",
                              "8/8/2p1p3/2P1p3/2ppQ3/1p6/8/K1k5 b - - 0 1",
                              "8/8/2p1p3/2P1p3/2pQp3/1p6/8/K1k5 b - - 0 1"});
}

TEST_CASE("king moves") {
  auto in = std::stringstream{"8/8/2p1p3/2PKp3/2ppp3/8/8/8 w - - 0 1"};
  position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{"8/8/2K1p3/2P1p3/2ppp3/8/8/8 b - - 0 1",
                              "8/8/2p1p3/2P1K3/2ppp3/8/8/8 b - - 0 1",
                              "8/8/2p1K3/2P1p3/2ppp3/8/8/8 b - - 0 1",
                              "8/8/2pKp3/2P1p3/2ppp3/8/8/8 b - - 1 1",
                              "8/8/2p1p3/2P1p3/2Kpp3/8/8/8 b - - 0 1",
                              "8/8/2p1p3/2P1p3/2ppK3/8/8/8 b - - 0 1"});
}

TEST_CASE("white castle") {
  auto in = std::stringstream{"8/8/8/8/8/p2ppp1p/P2PPP1P/R3K2R w KQkq - 0 1"};
  position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{// pawn captures
                              "8/8/8/8/8/p2Ppp1p/P2P1P1P/R3K2R b KQkq - 0 1",
                              "8/8/8/8/8/p2ppP1p/P2P1P1P/R3K2R b KQkq - 0 1",
                              "8/8/8/8/8/p2pPp1p/P3PP1P/R3K2R b KQkq - 0 1",
                              "8/8/8/8/8/p2pPp1p/P2PP2P/R3K2R b KQkq - 0 1",

                              // castle
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/R4RK1 b kq - 0 1",
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/2KR3R b kq - 0 1",

                              // king moves
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/R4K1R b kq - 0 1",
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/R2K3R b kq - 0 1",

                              // rook moves
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/1R2K2R b Kkq - 0 1",
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/2R1K2R b Kkq - 0 1",
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/3RK2R b Kkq - 0 1",
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/R3K1R1 b Qkq - 0 1",
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/R3KR2 b Qkq - 0 1"});
}

TEST_CASE("white cannot castle") {
  auto in = std::stringstream{"8/8/8/8/8/p2ppp1p/P2PPP1P/R3K2R w kq - 0 1"};
  position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{// pawn captures
                              "8/8/8/8/8/p2Ppp1p/P2P1P1P/R3K2R b kq - 0 1",
                              "8/8/8/8/8/p2ppP1p/P2P1P1P/R3K2R b kq - 0 1",
                              "8/8/8/8/8/p2pPp1p/P3PP1P/R3K2R b kq - 0 1",
                              "8/8/8/8/8/p2pPp1p/P2PP2P/R3K2R b kq - 0 1",

                              // king moves
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/R4K1R b kq - 1 1",
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/R2K3R b kq - 1 1",

                              // rook moves
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/1R2K2R b kq - 1 1",
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/2R1K2R b kq - 1 1",
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/3RK2R b kq - 1 1",
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/R3K1R1 b kq - 1 1",
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/R3KR2 b kq - 1 1"});
}

TEST_CASE("white castle with b1 attack") {
  auto in = std::stringstream{"8/8/8/8/8/n2ppp1p/P2PPP1P/R3K2R w KQkq - 0 1"};
  position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{// pawn captures
                              "8/8/8/8/8/n2Ppp1p/P2P1P1P/R3K2R b KQkq - 0 1",
                              "8/8/8/8/8/n2ppP1p/P2P1P1P/R3K2R b KQkq - 0 1",
                              "8/8/8/8/8/n2pPp1p/P3PP1P/R3K2R b KQkq - 0 1",
                              "8/8/8/8/8/n2pPp1p/P2PP2P/R3K2R b KQkq - 0 1",

                              // castle
                              "8/8/8/8/8/n2ppp1p/P2PPP1P/R4RK1 b kq - 0 1",
                              "8/8/8/8/8/n2ppp1p/P2PPP1P/2KR3R b kq - 0 1",

                              // king moves
                              "8/8/8/8/8/n2ppp1p/P2PPP1P/R4K1R b kq - 0 1",
                              "8/8/8/8/8/n2ppp1p/P2PPP1P/R2K3R b kq - 0 1",

                              // rook moves
                              "8/8/8/8/8/n2ppp1p/P2PPP1P/1R2K2R b Kkq - 0 1",
                              "8/8/8/8/8/n2ppp1p/P2PPP1P/2R1K2R b Kkq - 0 1",
                              "8/8/8/8/8/n2ppp1p/P2PPP1P/3RK2R b Kkq - 0 1",
                              "8/8/8/8/8/n2ppp1p/P2PPP1P/R3K1R1 b Qkq - 0 1",
                              "8/8/8/8/8/n2ppp1p/P2PPP1P/R3KR2 b Qkq - 0 1"});
}

TEST_CASE("white cannot castle with attack and block") {
  auto in =
      std::stringstream{"8/2r5/8/8/8/n2ppp1p/P2PPP1P/R3Kn1R w KQkq - 0 1"};
  position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{// pawn captures
                              "8/2r5/8/8/8/n2Ppp1p/P2P1P1P/R3Kn1R b KQkq - 0 1",
                              "8/2r5/8/8/8/n2ppP1p/P2P1P1P/R3Kn1R b KQkq - 0 1",
                              "8/2r5/8/8/8/n2pPp1p/P3PP1P/R3Kn1R b KQkq - 0 1",
                              "8/2r5/8/8/8/n2pPp1p/P2PP2P/R3Kn1R b KQkq - 0 1",

                              // king moves
                              "8/2r5/8/8/8/n2ppp1p/P2PPP1P/R4K1R b kq - 0 1",
                              "8/2r5/8/8/8/n2ppp1p/P2PPP1P/R2K1n1R b kq - 0 1",

                              // rook moves
                              "8/2r5/8/8/8/n2ppp1p/P2PPP1P/1R2Kn1R b Kkq - 0 1",
                              "8/2r5/8/8/8/n2ppp1p/P2PPP1P/2R1Kn1R b Kkq - 0 1",
                              "8/2r5/8/8/8/n2ppp1p/P2PPP1P/3RKn1R b Kkq - 0 1",
                              "8/2r5/8/8/8/n2ppp1p/P2PPP1P/R3KnR1 b Qkq - 0 1",
                              "8/2r5/8/8/8/n2ppp1p/P2PPP1P/R3KR2 b Qkq - 0 1"});
}

TEST_CASE("white cannot castle knight attack") {
  auto in = std::stringstream{"4k3/8/8/8/8/p2npp1p/P2PPP1P/R3K2R w KQ - 0 1"};
  position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{// pawn captures
                              "4k3/8/8/8/8/p2Ppp1p/P2P1P1P/R3K2R b KQ - 0 1",

                              // king moves
                              "4k3/8/8/8/8/p2npp1p/P2PPP1P/R4K1R b - - 0 1",
                              "4k3/8/8/8/8/p2npp1p/P2PPP1P/R2K3R b - - 0 1"});
}

TEST_CASE("white castle") {
  auto in = std::stringstream{"8/8/8/8/8/p2rpr1p/P2PpP1P/R3K2R w KQkq - 0 1"};
  position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{// pawn captures
                              "8/8/8/8/8/p2rPr1p/P3pP1P/R3K2R b KQkq - 0 1",
                              "8/8/8/8/8/p2rPr1p/P2Pp2P/R3K2R b KQkq - 0 1",

                              // king moves
                              "8/8/8/8/8/p2rpr1p/P2PKP1P/R6R b kq - 0 1",

                              // rook moves
                              "8/8/8/8/8/p2rpr1p/P2PpP1P/1R2K2R b Kkq - 0 1",
                              "8/8/8/8/8/p2rpr1p/P2PpP1P/2R1K2R b Kkq - 0 1",
                              "8/8/8/8/8/p2rpr1p/P2PpP1P/3RK2R b Kkq - 0 1",
                              "8/8/8/8/8/p2rpr1p/P2PpP1P/R3K1R1 b Qkq - 0 1",
                              "8/8/8/8/8/p2rpr1p/P2PpP1P/R3KR2 b Qkq - 0 1"});
}

TEST_CASE("make move from string") {
  auto p = test_position{start_position_fen};

  p.make_move("e2e3");
  p.make_move("e7e6");

  CHECK(p.to_fen() ==
        "rnbqkbnr/pppp1ppp/4p3/8/8/4P3/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
}

TEST_CASE("escape check") {
  constexpr auto const pos_fen = "8/k6P/8/8/8/p7/8/K6r w - - 0 2";
  auto in = std::stringstream{pos_fen};

  auto p = position{};
  in >> p;
  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{// pawn captures
                              "8/k6P/8/8/8/p7/K7/7r b - - 1 2"});
}

TEST_CASE("knight attacks by origin square") {
  auto const expected = std::string{R"(00000000
00000000
01010000
10001000
00000000
10001000
01010000
00000000
)"};
  CHECK(expected == bitboard_to_str(knight_attacks_by_origin_square[34]));
}

TEST_CASE("escape check big") {
  constexpr auto const pos_fen = "4q3/8/3n4/1K1Pp2r/8/8/8/1r3b1k w - e6 0 1";
  auto in = std::stringstream{pos_fen};

  auto p = position{};
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{"4q3/8/3n4/2KPp2r/8/8/8/1r3b1k b - - 1 1",
                              "4q3/8/3n4/K2Pp2r/8/8/8/1r3b1k b - - 1 1"});
}

TEST_CASE("escape check en passant") {
  constexpr auto const pos_fen = "8/1k6/8/1K2Pp1r/7r/1n6/8/8 w - - 1 1";
  auto in = std::stringstream{pos_fen};

  auto p = position{};
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{"8/1k6/4P3/1K3p1r/7r/1n6/8/8 b - - 0 1"});
}

TEST_CASE("detect check mate") {
  constexpr auto const scholars_mate_fen =
      "r1bqkb1r/pppp1Qpp/2n2n2/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 5 3";
  auto in = std::stringstream{scholars_mate_fen};

  auto p = position{};
  in >> p;

  CHECK(fen_strings_after_move(p).empty());
  //  CHECK(!is_valid_move<color::BLACK>(p, move{0U, 0U}));
}

TEST_CASE("detect non check mate") {
  auto in = std::stringstream{start_position_fen};

  auto p = position{};
  in >> p;

  CHECK(!fen_strings_after_move(p).empty());
  //  CHECK(is_valid_move<color::WHITE>(p, move{0U, 0U}));
}

TEST_CASE("white short castle with knight f2") {
  constexpr auto const fen =
      "rnbqkb1r/pppppppp/B7/8/8/8/PPPPNnPP/RNBQK2R w KQkq - 0 4";

  auto in = std::stringstream{fen};

  auto p = position{};
  in >> p;

  auto moves = std::set<std::string>{};
  auto move_list = std::array<move, max_moves>{};
  auto const end = generate_moves(p, &move_list[0]);
  for (auto const& m : utl::all(&move_list[0], end) | utl::iterable()) {
    moves.emplace(m.to_str());
  }
  CHECK(moves.find("e1g1") != std::end(moves));
}

TEST_CASE("black cannot castle rook captured") {
  auto p = test_position{
      "rnbqk2r/pppppp1p/5B1b/6p1/8/1P6/P1PPPPPP/RN1QKBNR w KQkq - 1 4"};
  p.make_move("f6h8");

  auto moves = std::set<std::string>{};
  auto move_list = std::array<move, max_moves>{};
  auto const end = generate_moves(p, &move_list[0]);
  for (auto const& m : utl::all(&move_list[0], end) | utl::iterable()) {
    moves.emplace(m.to_str());
  }

  CHECK(moves.find("e8g8") == std::end(moves));
}

TEST_CASE("en passant") {
  auto p = test_position{start_position_fen};

  p.make_move("e2e3");
  p.make_move("e7e5");
  p.make_move("e3e4");
  p.make_move("a7a6");
  p.make_move("d2d4");
  p.make_move("e5d4");
  p.make_move("c2c4");

  auto moves = std::set<std::string>{};
  auto move_list = std::array<move, max_moves>{};
  auto const end = generate_moves(p, &move_list[0]);
  for (auto const& m : utl::all(&move_list[0], end) | utl::iterable()) {
    moves.emplace(m.to_str());
  }
  CHECK(moves.find("d4c3") != std::end(moves));
}

TEST_CASE("capture pinner") {
  auto p = test_position{start_position_fen};

  p.make_move("a2a3");
  p.make_move("e7e5");
  p.make_move("d2d3");
  p.make_move("f8b4");
  p.make_move("a3b4");
  p.make_move("a7a5");

  auto moves = std::set<std::string>{};
  auto move_list = std::array<move, max_moves>{};
  auto const end = generate_moves(p, &move_list[0]);
  for (auto const& m : utl::all(&move_list[0], end) | utl::iterable()) {
    moves.emplace(m.to_str());
  }
  CHECK(moves.size() == 30);
  CHECK(moves.find("b4b5") != std::end(moves));
}

TEST_CASE("blocker moves on blocking line") {
  auto p = test_position{start_position_fen};

  p.make_move("d2d3");
  p.make_move("c7c5");
  p.make_move("e1d2");
  p.make_move("c5c4");
  p.make_move("d3c4");
  p.make_move("d7d5");

  auto moves = std::set<std::string>{};
  auto move_list = std::array<move, max_moves>{};
  auto const end = generate_moves(p, &move_list[0]);
  for (auto const& m : utl::all(&move_list[0], end) | utl::iterable()) {
    moves.emplace(m.to_str());
  }
  CHECK(moves.size() == 24);
  CHECK(moves.find("a2a3") != std::end(moves));
}

TEST_CASE("blocker captured by en passant") {
  auto p = test_position{start_position_fen};

  p.make_move("b2b4");
  p.make_move("e7e5");
  p.make_move("b4b5");
  p.make_move("e8e7");
  p.make_move("c1a3");
  p.make_move("c7c5");
  p.make_move("b5c6");

  auto moves = std::set<std::string>{};
  auto move_list = std::array<move, max_moves>{};
  auto const end = generate_moves(p, &move_list[0]);
  for (auto const& m : utl::all(&move_list[0], end) | utl::iterable()) {
    moves.emplace(m.to_str());
  }
  CHECK(moves.size() == 4);
  CHECK(moves.find("h7h6") == std::end(moves));
}

TEST_CASE("en passant capture removes two blockers") {
  auto p = test_position{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "};

  p.make_move("e2e4");

  auto moves = std::set<std::string>{};
  auto move_list = std::array<move, max_moves>{};
  auto const end = generate_moves(p, &move_list[0]);
  for (auto const& m : utl::all(&move_list[0], end) | utl::iterable()) {
    moves.emplace(m.to_str());
  }
  CHECK(moves.size() == 16);
  CHECK(moves.find("f4e3") == std::end(moves));
}

TEST_CASE("blocking pawn captures and clears line") {
  auto p = test_position{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "};

  p.make_move("b4b1");
  p.make_move("h4g5");
  p.make_move("b1f1");
  p.make_move("g5f6");
  p.make_move("e2e4");

  auto moves = std::set<std::string>{};
  auto move_list = std::array<move, max_moves>{};
  auto const end = generate_moves(p, &move_list[0]);
  for (auto const& m : utl::all(&move_list[0], end) | utl::iterable()) {
    moves.emplace(m.to_str());
  }
  CHECK(moves.size() == 24);
  CHECK(moves.find("f4e3") == std::end(moves));
}

TEST_CASE("en passant removes both pawns as blockers") {
  auto p = test_position{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "};

  p.make_move("e2e4");
  p.make_move("c7c5");
  p.make_move("g2g4");
  p.make_move("f4g3");
  p.make_move("e4e5");

  auto moves = std::set<std::string>{};
  auto move_list = std::array<move, max_moves>{};
  auto const end = generate_moves(p, &move_list[0]);
  for (auto const& m : utl::all(&move_list[0], end) | utl::iterable()) {
    moves.emplace(m.to_str());
  }
  CHECK(moves.size() == 4);
  CHECK(moves.find("d6d5") == std::end(moves));
}

TEST_CASE("blocker captured by king") {
  auto p = test_position{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "};

  p.make_move("g2g4");
  p.make_move("h4g4");
  p.make_move("a5a4");
  p.make_move("g4h4");
  p.make_move("a4a3");

  auto moves = std::set<std::string>{};
  auto move_list = std::array<move, max_moves>{};
  auto const end = generate_moves(p, &move_list[0]);
  for (auto const& m : utl::all(&move_list[0], end) | utl::iterable()) {
    moves.emplace(m.to_str());
  }
  CHECK(moves.size() == 16);
  CHECK(moves.find("f4f3") == std::end(moves));
}

TEST_CASE("promoted piece moves") {
  auto p = test_position{
      "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"};

  p.make_move("c4c5");
  p.make_move("b2a1b");
  p.make_move("a4b3");

  auto moves = std::set<std::string>{};
  auto move_list = std::array<move, max_moves>{};
  auto const end = generate_moves(p, &move_list[0]);
  for (auto const& m : utl::all(&move_list[0], end) | utl::iterable()) {
    moves.emplace(m.to_str());
  }
  CHECK(moves.find("a1b2") != std::end(moves));
}

TEST_CASE("castle into pin") {
  auto p = test_position{
      "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"};

  p.make_move("c4c5");
  p.make_move("b2a1b");
  p.make_move("d1c1");
  p.make_move("e8c8");
  p.make_move("c5b6");

  auto moves = std::set<std::string>{};
  auto move_list = std::array<move, max_moves>{};
  auto const end = generate_moves(p, &move_list[0]);
  for (auto const& m : utl::all(&move_list[0], end) | utl::iterable()) {
    moves.emplace(m.to_str());
  }
  CHECK(moves.find("c7b6") == std::end(moves));
}

TEST_CASE("checking piece becomes pinner after king move") {
  auto p = test_position{
      "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"};

  p.make_move("b4c5");
  p.make_move("a3b3");
  p.make_move("g1f2");
  p.make_move("b3e3");
  p.make_move("f2g3");
  p.make_move("a5b3");

  auto moves = std::set<std::string>{};
  auto move_list = std::array<move, max_moves>{};
  auto const end = generate_moves(p, &move_list[0]);
  for (auto const& m : utl::all(&move_list[0], end) | utl::iterable()) {
    moves.emplace(m.to_str());
  }
  CHECK(moves.find("f3d4") == std::end(moves));
}

TEST_CASE("pinner captured") {
  auto p = test_position{
      "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"};

  p.make_move("b4c5");
  p.make_move("e8c8");
  p.make_move("d1c1");
  p.make_move("b2c1q");
  p.make_move("f1c1");
  p.make_move("a3a2");

  auto moves = std::set<std::string>{};
  auto move_list = std::array<move, max_moves>{};
  auto const end = generate_moves(p, &move_list[0]);
  for (auto const& m : utl::all(&move_list[0], end) | utl::iterable()) {
    moves.emplace(m.to_str());
  }
  CHECK(moves.find("c5a3") == std::end(moves));
}

TEST_CASE("???") {
  auto p = test_position{
      "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"};

  p.make_move("f3d4");
  p.make_move("b6c5");
  p.make_move("g1f2");
  p.make_move("f6d5");
  p.make_move("h6f7");
  p.make_move("e8g8");

  auto moves = std::set<std::string>{};
  auto move_list = std::array<move, max_moves>{};
  auto const end = generate_moves(p, &move_list[0]);
  for (auto const& m : utl::all(&move_list[0], end) | utl::iterable()) {
    moves.emplace(m.to_str());
  }
  CHECK(moves.find("f7h8") == std::end(moves));
}

TEST_CASE("dfs startpos 5") {
  auto p = test_position{start_position_fen};
  CHECK(119060324 == dfs_rec(p, 0U, 6, nullptr));
}

TEST_CASE("kiwipete") {
  auto p = test_position{
      "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -"};
  CHECK(193690690 == dfs_rec(p, 0U, 5, nullptr));
}

TEST_CASE("position 3 - invalid en passant") {
  auto p = test_position{"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -"};
  CHECK(11030083 == dfs_rec(p, 0U, 6, nullptr));
}

TEST_CASE("position 4 - chaos") {
  auto p = test_position{
      "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"};
  CHECK(15833292 == dfs_rec(p, 0U, 5, nullptr));
}

TEST_CASE("position 5 - pin by castle") {
  auto p = test_position{
      "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8"};
  CHECK(89941194 == dfs_rec(p, 0U, 5, nullptr));
}

TEST_CASE("position 5 - pin by castle") {
  auto p = test_position{
      "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 "
      "10"};
  CHECK(164075551 == dfs_rec(p, 0U, 5, nullptr));
}