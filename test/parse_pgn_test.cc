#include "doctest/doctest.h"

#include <charconv>
#include <optional>
#include <vector>

#include "utl/parser/cstr.h"
#include "utl/raii.h"
#include "utl/verify.h"

#include "chessbot/pgn.h"
#include "chessbot/position.h"
#include "chessbot/util.h"

using namespace chessbot;

constexpr auto const test_pgn =
    R"([Event "Rated Classical tournament https://lichess.org/tournament/whc7Blcq"]
[Site "https://lichess.org/z0CDaIfk"]
[White "AJEDREZSUPERIOR"]
[Black "HighLevelUser"]
[Result "0-1"]
[UTCDate "2017.03.31"]
[UTCTime "22:06:32"]
[WhiteElo "1687"]
[BlackElo "1954"]
[WhiteRatingDiff "-4"]
[BlackRatingDiff "+5"]
[ECO "D04"]
[Opening "Queen's Pawn Game: Colle System"]
[TimeControl "600+10"]
[Termination "Normal"]

1. d4 { [%clk 0:10:00] } d5 { [%clk 0:05:00] } 2. Nf3 { [%clk 0:09:22] } Nf6 { [%clk 0:04:57] } 3. e3 { [%clk 0:09:18] } c5 { [%clk 0:04:54] } 4. c3 { [%clk 0:09:17] } Nc6 { [%clk 0:04:52] } 5. Bd3 { [%clk 0:09:16] } cxd4 { [%clk 0:04:49] } 6. cxd4 { [%clk 0:09:13] } Bg4 { [%clk 0:04:46] } 7. a3 { [%clk 0:09:11] } e5 { [%clk 0:04:43] } 8. Be2 { [%clk 0:09:09] } e4 { [%clk 0:04:37] } 9. Ng5 { [%clk 0:08:24] } h5 { [%clk 0:04:28] } 10. f4 { [%clk 0:08:07] } exf3 { [%clk 0:04:21] } 11. Bxf3 { [%clk 0:07:32] } Ne4 { [%clk 0:04:13] } 12. Nxe4 { [%clk 0:07:26] } dxe4 { [%clk 0:04:07] } 13. Bxg4 { [%clk 0:07:23] } hxg4 { [%clk 0:04:02] } 14. Qxg4 { [%clk 0:07:22] } Rh4 { [%clk 0:03:57] } 15. Qe2 { [%clk 0:06:59] } Qg5 { [%clk 0:03:42] } 16. g3 { [%clk 0:06:46] } Rh3 { [%clk 0:03:30] } 17. Nd2 { [%clk 0:06:42] } f5 { [%clk 0:03:25] } 18. Nc4 { [%clk 0:06:40] } O-O-O { [%clk 0:03:17] } 19. Bd2 { [%clk 0:06:31] } b5 { [%clk 0:03:10] } 20. Ne5 { [%clk 0:06:19] } Nxe5 { [%clk 0:03:06] } 21. dxe5 { [%clk 0:06:18] } a6 { [%clk 0:03:01] } 22. O-O-O { [%clk 0:06:00] } Rh6 { [%clk 0:02:49] } 23. Kb1 { [%clk 0:05:59] } Rc6 { [%clk 0:02:45] } 24. Rc1 { [%clk 0:05:53] } Kb7 { [%clk 0:02:41] } 25. Ba5 { [%clk 0:05:45] } Rdc8 { [%clk 0:02:32] } 26. Rc3 { [%clk 0:04:36] } Rxc3 { [%clk 0:02:23] } 27. Bxc3 { [%clk 0:04:33] } Bxa3 { [%clk 0:02:19] } 28. Qd2 { [%clk 0:04:02] } Bc5 { [%clk 0:02:02] } 29. Qd7+ { [%clk 0:03:44] } Rc7 { [%clk 0:01:58] } 30. Qd5+ { [%clk 0:03:38] } Kb6 { [%clk 0:01:55] } 31. Bd4 { [%clk 0:02:39] } Qe7 { [%clk 0:01:41] } 32. Rc1 { [%clk 0:02:25] } g5 { [%clk 0:01:17] } 33. b4 { [%clk 0:02:15] } Bxd4 { [%clk 0:01:09] } 34. Qxd4+ { [%clk 0:01:53] } Kb7 { [%clk 0:01:02] } 35. Qd5+ { [%clk 0:01:37] } Kb8 { [%clk 0:00:59] } 36. Qg8+ { [%clk 0:01:31] } Ka7 { [%clk 0:00:58] } 37. Rxc7+ { [%clk 0:01:10] } Qxc7 { [%clk 0:00:56] } 38. Qxg5 { [%clk 0:00:58] } Qxe5 { [%clk 0:00:55] } 39. Qd8 { [%clk 0:00:45] } Qc3 { [%clk 0:00:51] } 40. Qd4+ { [%clk 0:00:42] } Qxd4 { [%clk 0:00:49] } 41. exd4 { [%clk 0:00:41] } Kb6 { [%clk 0:00:41] } 42. Kc2 { [%clk 0:00:39] } a5 { [%clk 0:00:41] } 43. bxa5+ { [%clk 0:00:37] } Kxa5 { [%clk 0:00:40] } 44. Kc3 { [%clk 0:00:31] } b4+ { [%clk 0:00:39] } 45. Kb3 { [%clk 0:00:28] } e3 { [%clk 0:00:38] } 46. Kc2 { [%clk 0:00:19] } e2 { [%clk 0:00:37] } 47. Kd2 { [%clk 0:00:18] } b3 { [%clk 0:00:37] } 48. d5 { [%clk 0:00:15] } b2 { [%clk 0:00:37] } 49. d6 { [%clk 0:00:14] } b1=Q { [%clk 0:00:36] } 50. d7 { [%clk 0:00:12] } e1=Q# { [%clk 0:00:34] } 0-1)";

TEST_CASE("pgn parse pgn") {
  auto pgn = utl::cstr{test_pgn};
  auto const h = parse_header(pgn);
  CHECK(h.elo_white_ == 1687);
  CHECK(h.elo_black_ == 1954);
  CHECK(h.start_time_ == 600);
  CHECK(h.time_increment_ == 10);
  CHECK(h.site_ == "https://lichess.org/z0CDaIfk");
  REQUIRE(!pgn.empty());
  CHECK(pgn[0] == '1');
}

TEST_CASE("pgn parse move") {
  auto pgn = utl::cstr{"d4 { [%clk 0:10:00] }"};
  auto const m = parse_move(pgn);
  CHECK(m.to_ == name_to_square("d4"));
  CHECK(pgn.empty());
}

TEST_CASE("pgn parse knight move 2") {
  auto pgn = utl::cstr{"Nad4 { [%clk 0:10:00] }"};
  auto const m = parse_move(pgn);
  CHECK(m.to_ == name_to_square("d4"));
  CHECK(m.from_file_ == 'a');
  CHECK(m.piece_ == piece_type::KNIGHT);
  CHECK(pgn.empty());
}

TEST_CASE("pgn parse check") {
  auto pgn = utl::cstr{"Nad4+ { [%clk 0:10:00] }"};
  auto const m = parse_move(pgn);
  CHECK(m.to_ == name_to_square("d4"));
  CHECK(m.from_file_ == 'a');
  CHECK(m.piece_ == piece_type::KNIGHT);
  CHECK(pgn.empty());
}

TEST_CASE("pgn parse mate") {
  auto pgn = utl::cstr{"Nad4# { [%clk 0:10:00] }"};
  auto const m = parse_move(pgn);
  CHECK(m.to_ == name_to_square("d4"));
  CHECK(m.from_file_ == 'a');
  CHECK(m.piece_ == piece_type::KNIGHT);
  CHECK(pgn.empty());
}

TEST_CASE("pgn parse promotion") {
  auto pgn = utl::cstr{"e1=Q# { [%clk 0:10:00] }"};
  auto const m = parse_move(pgn);
  CHECK(m.to_ == name_to_square("e1"));
  CHECK(m.piece_ == PAWN);
  CHECK(m.promotion_piece_type_ == promotion_piece_type::QUEEN);
  CHECK(pgn.empty());
}

TEST_CASE("pgn short castle") {
  auto pgn = utl::cstr{"O-O { [%clk 0:10:00] }"};
  auto const m = parse_move(pgn);
  CHECK(m.castle_ == game::move::castle::SHORT);
  CHECK(pgn.empty());
}

TEST_CASE("pgn long castle") {
  auto pgn = utl::cstr{"O-O-O { [%clk 0:10:00] }"};
  auto const m = parse_move(pgn);
  CHECK(m.castle_ == game::move::castle::LONG);
  CHECK(pgn.empty());
}

TEST_CASE("pgn parse 2 moves") {
  auto pgn =
      utl::cstr{"1. d4 { [%clk 0:10:00] } d5 { [%clk 0:10:00] } 1/2-1/2"};
  auto const m = parse_moves(pgn);
  REQUIRE(m.size() == 2);
  CHECK(m[0].to_ == name_to_square("d4"));
  CHECK(m[1].to_ == name_to_square("d5"));
  CHECK(pgn.empty());
}

TEST_CASE("pgn parse game") {
  auto pgn = utl::cstr{test_pgn};
  auto const g = parse_pgn(pgn);

  CHECK(g.header_.elo_white_ == 1687);
  CHECK(g.header_.elo_black_ == 1954);
  CHECK(g.header_.start_time_ == 600);
  CHECK(g.header_.time_increment_ == 10);

  auto const& n0 = g.moves_[g.moves_.size() - 2];
  auto const& n1 = g.moves_[g.moves_.size() - 1];
  CHECK(name_to_square("d7") == n0.to_);
  CHECK(name_to_square("e1") == n1.to_);
  CHECK(n1.promotion_piece_type_ == promotion_piece_type::QUEEN);
}

TEST_CASE("pgn make move") {
  auto p = position::from_fen(start_position_fen);
  auto pgn = utl::cstr{"Nf3 { [%clk 0:10:00] }"};
  auto const m = parse_move(pgn);
  p.make_pgn_move(m, nullptr);
  CHECK(m.to_ == name_to_square("f3"));
  CHECK(m.piece_ == piece_type::KNIGHT);
  CHECK(p.to_fen() ==
        "rnbqkbnr/pppppppp/8/8/8/5N2/PPPPPPPP/RNBQKB1R b KQkq - 1 1");
  CHECK(pgn.empty());
}

TEST_CASE("pgn make multiple moves") {
  auto p = position::from_fen(start_position_fen);
  auto pgn = utl::cstr{
      R"(1. Nf3 { [%clk 0:10:00] } Nf6 { [%clk 0:10:00] } 2. e4 { [%clk 0:10:00] } e5 { [%clk 0:10:00] } 3. Be2 { [%clk 0:10:00] } Be7 { [%clk 0:10:00] } 4. O-O { [%clk 0:10:00] } 1-0)"};
  for (auto const& m : parse_moves(pgn)) {
    p.make_pgn_move(m, nullptr);
  }
  CHECK(p.to_fen() ==
        "rnbqk2r/ppppbppp/5n2/4p3/4P3/5N2/PPPPBPPP/RNBQ1RK1 b kq - 0 4");
  CHECK(pgn.empty());
}

TEST_CASE("pgn make multiple moves from square set") {
  auto p = position::from_fen(start_position_fen);
  auto pgn = utl::cstr{
      R"(1. e4 { [%clk 0:10:00] } d5 { [%clk 0:10:00] } 2. exd5 { [%clk 0:10:00] } 1-0)"};
  for (auto const& m : parse_moves(pgn)) {
    p.make_pgn_move(m, nullptr);
  }
  CHECK(p.to_fen() ==
        "rnbqkbnr/ppp1pppp/8/3P4/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2");
  CHECK(pgn.empty());
}

TEST_CASE("pgn multiple knights can move to same square") {
  auto p = position::from_fen(
      "rnbqkb1r/ppp2ppp/4pn2/3P4/8/8/PPPPNPPP/RNBQKB1R w KQkq - 0 4");
  auto pgn = utl::cstr{R"(1. Nbc3 { [%clk 0:10:00] } 1-0)"};
  for (auto const& m : parse_moves(pgn)) {
    p.make_pgn_move(m, nullptr);
  }
  CHECK(p.to_fen() ==
        "rnbqkb1r/ppp2ppp/4pn2/3P4/8/2N5/PPPPNPPP/R1BQKB1R b KQkq - 1 4");
  CHECK(pgn.empty());
}

TEST_CASE("pgn from square given by rank") {
  auto p = position::from_fen("2k5/8/2K5/R7/8/8/R7/8 w - - 0 1");
  auto pgn = utl::cstr{R"(1. R2a4 { [%clk 0:10:00] } 1-0)"};
  for (auto const& m : parse_moves(pgn)) {
    p.make_pgn_move(m, nullptr);
  }
  CHECK(p.to_fen() == "2k5/8/2K5/R7/R7/8/8/8 b - - 1 1");
  CHECK(pgn.empty());
}

TEST_CASE("pgn doubled pawn moves") {
  auto p = position::from_fen("2k5/8/2K5/R7/8/2P5/R1P5/8 w - - 0 1");
  auto pgn = utl::cstr{R"(1. c4 { [%clk 0:10:00] } 1-0)"};
  for (auto const& m : parse_moves(pgn)) {
    p.make_pgn_move(m, nullptr);
  }
  CHECK(p.to_fen() == "2k5/8/2K5/R7/2P5/8/R1P5/8 b - - 0 1");
  CHECK(pgn.empty());
}

TEST_CASE("pgn double pawn move") {
  auto p = position::from_fen("2k5/6p1/2K5/R7/2P5/8/R1P5/8 b - - 0 1");
  auto pgn = utl::cstr{R"(1. g5 { [%clk 0:10:00] } 1-0)"};
  for (auto const& m : parse_moves(pgn)) {
    p.make_pgn_move(m, nullptr);
  }
  CHECK(p.to_fen() == "2k5/8/2K5/R5p1/2P5/8/R1P5/8 w - g6 0 2");
  CHECK(pgn.empty());
}

TEST_CASE("pgn promotion") {
  auto p = position::from_fen("2k1bb2/4PPPP/2K5/8/8/8/8/8 w - - 0 1");
  auto pgn = utl::cstr{
      R"(1. exf8=N { [%clk 0:10:00] } Kb8 { [%clk 0:10:00] } 2. fxe8=B { [%clk 0:10:00] } Kc8 { [%clk 0:10:00] } 3. g8=R { [%clk 0:10:00] } Kb8 { [%clk 0:10:00] } 4. h8=Q { [%clk 0:10:00] } 1-0)"};
  for (auto const& m : parse_moves(pgn)) {
    p.make_pgn_move(m, nullptr);
  }
  CHECK(p.to_fen() == "1k2BNRQ/8/2K5/8/8/8/8/8 b - - 0 4");
  CHECK(pgn.empty());
}

TEST_CASE("pgn play a game") {
  auto p = position::from_fen(start_position_fen);
  auto pgn = utl::cstr{test_pgn};
  parse_header(pgn);
  for (auto const& m : parse_moves(pgn)) {
    p.make_pgn_move(m, nullptr);
  }
  CHECK(p.to_fen() == "8/3P4/8/k4p2/8/6P1/3K3P/1q2q3 w - - 0 51");
}

TEST_CASE("pgn black move numbers") {
  auto pgn = utl::cstr{
      R"([Event "Rated Classical tournament https://lichess.org/tournament/whc7Blcq"]
[Site "https://lichess.org/lHKEYATm"]
[White "madskillz"]
[Black "Pavvell"]
[Result "1-0"]
[UTCDate "2017.03.31"]
[UTCTime "22:00:02"]
[WhiteElo "2207"]
[BlackElo "2311"]
[WhiteRatingDiff "+20"]
[BlackRatingDiff "-16"]
[ECO "A74"]
[Opening "Benoni Defense: Classical Variation, Full Line"]
[TimeControl "600+0"]
[Termination "Normal"]

1. d4?! { [%eval 0.27] [%clk 0:05:00] } 1... Nf6?? { [%eval 0.29] [%clk 0:05:00] } 2. c4!! { [%eval 0.25] [%clk 0:04:59] } 2... c5!? { [%eval 0.36] [%clk 0:04:59] } 3. O-O-O+!! 1-0)"};

  auto const g = parse_pgn(pgn);
  CHECK(g.moves_.size() == 5);
}

TEST_CASE("pgn pinned") {
  auto pgn = utl::cstr{R"([Event "Rated Blitz game"]
[Site "https://lichess.org/mJUymeRl"]
[Date "2017.03.31"]
[White "frefafi35"]
[Black "Petrovich63"]
[Result "1-0"]
[UTCDate "2017.03.31"]
[UTCTime "22:04:04"]
[WhiteElo "2152"]
[BlackElo "2231"]
[WhiteRatingDiff "+103"]
[BlackRatingDiff "-10"]
[Variant "Standard"]
[TimeControl "180+3"]
[ECO "B12"]
[Opening "Caro-Kann Defense: Advance Variation, Botvinnik-Carls Defense"]
[Termination "Time forfeit"]
[Annotator "lichess.org"]

1. e4 c6 2. d4 d5 3. e5 c5 { B12 Caro-Kann Defense: Advance Variation, Botvinnik-Carls Defense } 4. dxc5 Nc6 5. Bb5 Qa5+?! { (-0.09 → 0.53) Inaccuracy. e6 was best. } (5... e6 6. Be3 Ne7 7. Nf3 Nf5 8. Bg5 Qa5+ 9. Nc3 Bxc5 10. O-O h6 11. Bf4 O-O 12. a3) 6. Nc3 e6 7. Be3 Ne7 8. a3 Nf5 9. Nf3 Nxe3 10. fxe3 Qc7 11. e4 Bxc5?! { (0.68 → 1.54) Inaccuracy. a6 was best. } (11... a6 12. exd5 axb5 13. dxc6 Bxc5 14. b4 Be7 15. Nxb5 Qxc6 16. Nd6+ Bxd6 17. exd6 Ra6 18. Qd3) 12. exd5 exd5 13. Qxd5 Bb6 14. Qe4 O-O? { (1.65 → 3.25) Mistake. Be6 was best. } (14... Be6 15. O-O-O Rd8 16. Rxd8+ Qxd8 17. Bxc6+ bxc6 18. Qxc6+ Qd7 19. Qe4 O-O 20. Rd1 Qc8 21. Nd4) 15. Nd5 Qd8 16. Bd3 g6 17. Nf6+ Kg7 18. O-O-O Be6?! { (2.59 → 3.61) Inaccuracy. Qe7 was best. } (18... Qe7 19. g4) 19. h4 Qe7?! { (3.41 → 4.89) Inaccuracy. Bd4 was best. } (19... Bd4 20. Bb5 Bf5 21. Qe2 Qb6 22. g4 Be6 23. h5 Rad8 24. hxg6 Be3+ 25. Kb1 Rxd1+ 26. Rxd1) 20. h5 Rh8 21. Qf4 Nxe5? { (4.20 → 9.84) Mistake. Rad8 was best. } (21... Rad8) 22. h6+ Kf8 23. Qxe5 { White wins on time. } 1-0)"};

  auto const g = parse_pgn(pgn);
  CHECK(g.moves_.size() == 47);

  auto p = position::from_fen(start_position_fen);
  for (auto const& m : g.moves_) {
    p.make_pgn_move(m, nullptr);
  }
  CHECK(p.to_fen() ==
        "r4k1r/pp2qp1p/1b2bNpP/4Q3/8/P2B1N2/1PP3P1/2KR3R b - - 0 23");
}