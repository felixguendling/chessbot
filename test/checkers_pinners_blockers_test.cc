#include "doctest/doctest.h"

#include "utl/pipes.h"

#include "chessbot/constants.h"
#include "chessbot/generate_moves.h"
#include "chessbot/position.h"

using namespace chessbot;

TEST_CASE("fen initialization") {
  auto p = position::from_fen(start_position_fen);
  CHECK(p.checkers_[color::WHITE] == bitboard{0});
  CHECK(p.checkers_[color::BLACK] == bitboard{0});
  CHECK(p.pinners_[color::WHITE] == bitboard{0});
  CHECK(p.pinners_[color::BLACK] == bitboard{0});
  CHECK(p.blockers_for_king_[color::WHITE] == bitboard{0});
  CHECK(p.blockers_for_king_[color::BLACK] == bitboard{0});
}

std::vector<std::pair<position, state_info>> after_move(position const& p) {
  auto moves = std::array<move, max_moves>{};
  auto const begin = &moves[0];
  auto const end = generate_moves(p, begin);

  auto r = std::vector<std::pair<position, state_info>>{};
  for (auto const& m : utl::all(begin, end) | utl::iterable()) {
    auto copy = p;
    r.emplace_back(copy, copy.make_move(m, nullptr));
  }
  return r;
}

TEST_CASE("ckeckers update") {
  auto const p = position::from_fen(
      "qb4n1/pPP1pbp1/4pprp/PK5P/4R1Rp/2p2p1P/3pnk1p/3R3R b - - 0 1");
  CHECK(p.checkers_[color::WHITE] == bitboard{0});
  CHECK(p.checkers_[color::BLACK] == bitboard{0});
  CHECK(p.pinners_[color::WHITE] == bitboard{0});
  CHECK(p.pinners_[color::BLACK] == bitboard{0});
  CHECK(p.blockers_for_king_[color::BLACK] == bitboard{0});
  CHECK(p.blockers_for_king_[color::WHITE] == bitboard{0});

  for (auto const& [next_p, state] : after_move(p)) {
    switch (state.last_move_.to()) {
      case rank_file_to_bitboard(R7, FB):
        CHECK(next_p.checkers_[color::WHITE] == rank_file_to_bitboard(R7, FB));
        break;
      case rank_file_to_bitboard(R8, FE):
        CHECK(next_p.checkers_[color::WHITE] == rank_file_to_bitboard(R8, FE));
        break;
      case rank_file_to_bitboard(R4, FD):
        CHECK(next_p.checkers_[color::WHITE] == rank_file_to_bitboard(R4, FD));
        break;
      case rank_file_to_bitboard(R5, FG):
        CHECK(next_p.checkers_[color::WHITE] == rank_file_to_bitboard(R5, FG));
        break;
      case rank_file_to_bitboard(R6, FA):
        CHECK(next_p.checkers_[color::WHITE] == rank_file_to_bitboard(R6, FA));
        break;
      default: CHECK(next_p.checkers_[color::WHITE] == bitboard{0}); break;
    }
  }
}

TEST_CASE("double check rook") {
  auto const p = position::from_fen("8/8/2K5/4p3/2n5/5k2/8/2r5 b - - 0 1");
  CHECK(p.checkers_[color::BLACK] == bitboard{0});
  CHECK(p.pinners_[color::BLACK] == bitboard{0});
  CHECK(p.blockers_for_king_[color::BLACK] == bitboard{0});
  CHECK(p.checkers_[color::WHITE] == bitboard{0});
  CHECK(p.pinners_[color::WHITE] == rank_file_to_bitboard(R1, FC));
  CHECK(p.blockers_for_king_[color::WHITE] == rank_file_to_bitboard(R4, FC));

  for (auto const& [next_p, state] : after_move(p)) {
    if (state.last_move_.from() == rank_file_to_bitboard(R4, FC)) {
      if (state.last_move_.to() == rank_file_to_bitboard(R5, FA)) {
        CHECK(next_p.checkers_[color::WHITE] ==
              (rank_file_to_bitboard(R1, FC) | rank_file_to_bitboard(R5, FA)));
      } else {
        CHECK(next_p.checkers_[color::WHITE] == rank_file_to_bitboard(R1, FC));
      }
    } else {
      CHECK(next_p.checkers_[color::WHITE] == 0);
    }

    if (state.last_move_.from() == rank_file_to_bitboard(R1, FC) &&
        (state.last_move_.to() & full_file_bitboard(FC))) {
      CHECK(next_p.blockers_for_king_[color::WHITE] ==
            rank_file_to_bitboard(R4, FC));
      CHECK(next_p.pinners_[color::WHITE] == state.last_move_.to());
    }
  }
}

TEST_CASE("double check bishop") {
  auto const p = position::from_fen("8/5K2/3p4/8/2n5/5k2/b7/8 b - - 0 1");
  CHECK(p.checkers_[color::BLACK] == bitboard{0});
  CHECK(p.pinners_[color::BLACK] == bitboard{0});
  CHECK(p.blockers_for_king_[color::BLACK] == bitboard{0});
  CHECK(p.checkers_[color::WHITE] == bitboard{0});
  CHECK(p.pinners_[color::WHITE] == rank_file_to_bitboard(R2, FA));
  CHECK(p.blockers_for_king_[color::WHITE] == rank_file_to_bitboard(R4, FC));

  for (auto const& [next_p, state] : after_move(p)) {
    if (state.last_move_.from() == rank_file_to_bitboard(R4, FC)) {
      if (state.last_move_.to() == rank_file_to_bitboard(R5, FE)) {
        CHECK(next_p.checkers_[color::WHITE] ==
              (rank_file_to_bitboard(R2, FA) | rank_file_to_bitboard(R5, FE)));
      } else {
        CHECK(next_p.checkers_[color::WHITE] == rank_file_to_bitboard(R2, FA));
      }
    } else {
      CHECK(next_p.checkers_[color::WHITE] == 0);
    }

    if (state.last_move_.from() == rank_file_to_bitboard(R2, FA) &&
        (state.last_move_.to() & rank_file_to_bitboard(R3, FB))) {
      CHECK(next_p.blockers_for_king_[color::WHITE] ==
            rank_file_to_bitboard(R4, FC));
      CHECK(next_p.pinners_[color::WHITE] == state.last_move_.to());
    }

    if (state.last_move_.from() == rank_file_to_bitboard(R2, FA) &&
        (state.last_move_.to() & rank_file_to_bitboard(R1, FB))) {
      CHECK(next_p.blockers_for_king_[color::WHITE] == 0U);
      CHECK(next_p.pinners_[color::WHITE] == 0U);
    }
  }
}

TEST_CASE("blocker and pinner becomes pinner") {
  auto const p = position::from_fen("8/8/2kpRp1R/8/8/4K3/8/8 w - - 0 1");
  CHECK(p.checkers_[color::WHITE] == bitboard{0});
  CHECK(p.pinners_[color::WHITE] == bitboard{0});
  CHECK(p.blockers_for_king_[color::WHITE] == bitboard{0});
  CHECK(p.checkers_[color::BLACK] == bitboard{0});
  CHECK(p.pinners_[color::BLACK] ==
        (rank_file_to_bitboard(R6, FE) | rank_file_to_bitboard(R6, FH)));
  CHECK(p.blockers_for_king_[color::BLACK] ==
        (rank_file_to_bitboard(R6, FD) | rank_file_to_bitboard(R6, FE) |
         rank_file_to_bitboard(R6, FF)));

  auto const after_move_p = after_move(p);
  CHECK(after_move_p.size() == 23);
  for (auto const& [next_p, state] : after_move_p) {
    if (state.last_move_.to() == rank_file_to_bitboard(R6, FD)) {
      CHECK(next_p.checkers_[color::BLACK] == rank_file_to_bitboard(R6, FD));
    } else {
      CHECK(next_p.checkers_[color::BLACK] == 0);
    }

    if (state.last_move_.from() == rank_file_to_bitboard(R6, FH)) {
      CHECK(next_p.checkers_[color::BLACK] == 0);
      if (state.last_move_.to() != rank_file_to_bitboard(R6, FG)) {
        CHECK((next_p.blockers_for_king_[color::BLACK] &
               rank_file_to_bitboard(R6, FF)) == 0);
        if (state.last_move_.to() == rank_file_to_bitboard(R6, FF)) {
          CHECK(
              next_p.blockers_for_king_[color::BLACK] ==
              (rank_file_to_bitboard(R6, FE) | rank_file_to_bitboard(R6, FD)));
          CHECK(
              next_p.pinners_[color::BLACK] ==
              (rank_file_to_bitboard(R6, FF) | rank_file_to_bitboard(R6, FE)));
        } else {
          CHECK(next_p.blockers_for_king_[color::BLACK] ==
                rank_file_to_bitboard(R6, FD));
          CHECK(next_p.pinners_[color::BLACK] == rank_file_to_bitboard(R6, FE));
        }
      } else {
        CHECK(next_p.pinners_[color::BLACK] ==
              (rank_file_to_bitboard(R6, FE) | rank_file_to_bitboard(R6, FG)));
        CHECK(next_p.blockers_for_king_[color::BLACK] ==
              (rank_file_to_bitboard(R6, FE) | rank_file_to_bitboard(R6, FD) |
               rank_file_to_bitboard(R6, FF)));
      }
    } else if (state.last_move_.from() == rank_file_to_bitboard(R6, FE)) {
      if (state.last_move_.to() == rank_file_to_bitboard(R6, FF)) {
        CHECK(next_p.blockers_for_king_[color::BLACK] ==
              (rank_file_to_bitboard(R6, FD) | rank_file_to_bitboard(R6, FF)));
      } else if (state.last_move_.to() & full_file_bitboard(FE)) {
        CHECK(next_p.pinners_[color::BLACK] == rank_file_to_bitboard(R6, FH));
        CHECK(next_p.blockers_for_king_[color::BLACK] ==
              (rank_file_to_bitboard(R6, FD) | rank_file_to_bitboard(R6, FF)));
      }
    }
  }
}

TEST_CASE("non-pinner becomes pinner") {
  auto const p = position::from_fen("8/8/2kp4/8/8/4K3/8/7R w - - 0 1");

  auto const after_move_p = after_move(p);
  CHECK(after_move_p.size() == 22);
  for (auto const& [next_p, state] : after_move_p) {
    if (state.last_move_.to() == rank_file_to_bitboard(R6, FH)) {
      CHECK(next_p.pinners_[color::BLACK] == rank_file_to_bitboard(R6, FH));
      CHECK(next_p.blockers_for_king_[color::BLACK] ==
            rank_file_to_bitboard(R6, FD));
    } else {
      CHECK(next_p.pinners_[color::BLACK] == 0);
      CHECK(next_p.blockers_for_king_[color::BLACK] == 0);
    }
  }
}
TEST_CASE("pin to save the king") {
  auto const p = position::from_fen("8/8/2k3Q1/8/6b1/4K3/8/8 b - - 0 1");

  auto const after_move_p = after_move(p);
  CHECK(after_move_p.size() == 7);
  for (auto const& [next_p, state] : after_move_p) {
    CHECK(next_p.pinners_[color::WHITE] == 0);
    CHECK(next_p.blockers_for_king_[color::WHITE] == 0);
    if (state.last_move_.to() == rank_file_to_bitboard(R6, FE)) {
      CHECK(next_p.pinners_[color::BLACK] == rank_file_to_bitboard(R6, FG));
      CHECK(next_p.blockers_for_king_[color::BLACK] ==
            rank_file_to_bitboard(R6, FE));
    } else {
      CHECK(next_p.pinners_[color::BLACK] == 0);
      CHECK(next_p.blockers_for_king_[color::BLACK] == 0);
    }
  }
}
