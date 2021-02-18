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

TEST_CASE("bishop moves") {
  auto in = std::stringstream{"8/1p2p3/4P3/3B4/2p1p3/8/8/8 w - - 0 1"};
  chessbot::position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{"8/1B2p3/4P3/8/2p1p3/8/8/8 b - - 0 1",
                              "8/1p2p3/4P3/8/2B1p3/8/8/8 b - - 0 1",
                              "8/1p2p3/4P3/8/2p1B3/8/8/8 b - - 0 1",
                              "8/1p2p3/2B1P3/8/2p1p3/8/8/8 b - - 0 1"});
}

TEST_CASE("rook moves") {
  auto in = std::stringstream{"8/3p4/1p6/1P1R2p1/3p4/8/8/8 w - - 0 1"};
  chessbot::position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{"8/3p4/1p1R4/1P4p1/3p4/8/8/8 b - - 0 1",
                              "8/3p4/1p6/1P4p1/3R4/8/8/8 b - - 0 1",
                              "8/3R4/1p6/1P4p1/3p4/8/8/8 b - - 0 1",
                              "8/3p4/1p6/1PR3p1/3p4/8/8/8 b - - 0 1",
                              "8/3p4/1p6/1P2R1p1/3p4/8/8/8 b - - 0 1",
                              "8/3p4/1p6/1P3Rp1/3p4/8/8/8 b - - 0 1",
                              "8/3p4/1p6/1P4R1/3p4/8/8/8 b - - 0 1"});
}

TEST_CASE("rook moves edge") {
  auto in = std::stringstream{"8/8/8/8/8/8/7p/5K1R w - - 0 1"};
  chessbot::position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{
            // rook moves
            "8/8/8/8/8/8/7R/5K2 b - - 0 1", "8/8/8/8/8/8/7p/5KR1 b - - 0 1",

            // king moves
            "8/8/8/8/8/8/7p/4K2R b - - 0 1", "8/8/8/8/8/8/7p/6KR b - - 0 1",
            "8/8/8/8/8/8/5K1p/7R b - - 0 1", "8/8/8/8/8/8/4K2p/7R b - - 0 1",
            "8/8/8/8/8/8/6Kp/7R b - - 0 1"});
}

TEST_CASE("queen moves") {
  auto in = std::stringstream{"8/8/2p1p3/2PQp3/2ppp3/8/8/8 w - - 0 1"};
  chessbot::position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{"8/8/2p1p3/2P1Q3/2ppp3/8/8/8 b - - 0 1",
                              "8/8/2Q1p3/2P1p3/2ppp3/8/8/8 b - - 0 1",
                              "8/8/2p1Q3/2P1p3/2ppp3/8/8/8 b - - 0 1",
                              "8/8/2pQp3/2P1p3/2ppp3/8/8/8 b - - 0 1",
                              "8/8/2p1p3/2P1p3/2Qpp3/8/8/8 b - - 0 1",
                              "8/8/2p1p3/2P1p3/2ppQ3/8/8/8 b - - 0 1",
                              "8/8/2p1p3/2P1p3/2pQp3/8/8/8 b - - 0 1",
                              "8/3Q4/2p1p3/2P1p3/2ppp3/8/8/8 b - - 0 1",
                              "3Q4/8/2p1p3/2P1p3/2ppp3/8/8/8 b - - 0 1"});
}

TEST_CASE("king moves") {
  auto in = std::stringstream{"8/8/2p1p3/2PKp3/2ppp3/8/8/8 w - - 0 1"};
  chessbot::position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{
            "8/8/2p1p3/2P1K3/2ppp3/8/8/8 b - - 0 1",
            "8/8/2K1p3/2P1p3/2ppp3/8/8/8 b - - 0 1",
            "8/8/2p1K3/2P1p3/2ppp3/8/8/8 b - - 0 1",
            "8/8/2pKp3/2P1p3/2ppp3/8/8/8 b - - 0 1",
            "8/8/2p1p3/2P1p3/2Kpp3/8/8/8 b - - 0 1",
            "8/8/2p1p3/2P1p3/2ppK3/8/8/8 b - - 0 1",
            "8/8/2p1p3/2P1p3/2pKp3/8/8/8 b - - 0 1",
        });
}

TEST_CASE("white castle") {
  auto in = std::stringstream{"8/8/8/8/8/p2ppp1p/P2PPP1P/R3K2R w KQkq - 0 1"};
  chessbot::position p;
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
  chessbot::position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{// pawn captures
                              "8/8/8/8/8/p2Ppp1p/P2P1P1P/R3K2R b kq - 0 1",
                              "8/8/8/8/8/p2ppP1p/P2P1P1P/R3K2R b kq - 0 1",
                              "8/8/8/8/8/p2pPp1p/P3PP1P/R3K2R b kq - 0 1",
                              "8/8/8/8/8/p2pPp1p/P2PP2P/R3K2R b kq - 0 1",

                              // king moves
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/R4K1R b kq - 0 1",
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/R2K3R b kq - 0 1",

                              // rook moves
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/1R2K2R b kq - 0 1",
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/2R1K2R b kq - 0 1",
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/3RK2R b kq - 0 1",
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/R3K1R1 b kq - 0 1",
                              "8/8/8/8/8/p2ppp1p/P2PPP1P/R3KR2 b kq - 0 1"});
}

TEST_CASE("white castle with b1 attack") {
  auto in = std::stringstream{"8/8/8/8/8/n2ppp1p/P2PPP1P/R3K2R w KQkq - 0 1"};
  chessbot::position p;
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
  chessbot::position p;
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
  auto in = std::stringstream{"8/8/8/8/8/p2npp1p/P2PPP1P/R3K2R w KQkq - 0 1"};
  chessbot::position p;
  in >> p;

  CHECK(fen_strings_after_move(p) ==
        std::set<std::string>{// pawn captures
                              "8/8/8/8/8/p2Ppp1p/P2P1P1P/R3K2R b KQkq - 0 1",
                              "8/8/8/8/8/p2npP1p/P2P1P1P/R3K2R b KQkq - 0 1",
                              "8/8/8/8/8/p2nPp1p/P3PP1P/R3K2R b KQkq - 0 1",
                              "8/8/8/8/8/p2nPp1p/P2PP2P/R3K2R b KQkq - 0 1",

                              // king moves
                              "8/8/8/8/8/p2npp1p/P2PPP1P/R4K1R b kq - 0 1",
                              "8/8/8/8/8/p2npp1p/P2PPP1P/R2K3R b kq - 0 1",

                              // rook moves
                              "8/8/8/8/8/p2npp1p/P2PPP1P/1R2K2R b Kkq - 0 1",
                              "8/8/8/8/8/p2npp1p/P2PPP1P/2R1K2R b Kkq - 0 1",
                              "8/8/8/8/8/p2npp1p/P2PPP1P/3RK2R b Kkq - 0 1",
                              "8/8/8/8/8/p2npp1p/P2PPP1P/R3K1R1 b Qkq - 0 1",
                              "8/8/8/8/8/p2npp1p/P2PPP1P/R3KR2 b Qkq - 0 1"});
}
