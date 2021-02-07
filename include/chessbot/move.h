#pragma once

#include <cmath>

#include "chessbot/position.h"

#include "cista/bit_counting.h"

namespace chessbot {

struct move {
  explicit move(bitboard from, bitboard to)
      : from_field_{static_cast<uint16_t>(cista::trailing_zeros(from))},
        to_field_{static_cast<uint16_t>(cista::trailing_zeros(to))} {}

  bitboard from() const { return rank_file_to_bitboard(0, from_field_); }
  bitboard to() const { return rank_file_to_bitboard(0, to_field_); }

  uint16_t from_field_ : 6;
  uint16_t to_field_ : 6;
  uint16_t promotion_piece_type_ : 2;
  uint16_t special_move_ : 2;
};

inline constexpr bitboard full_rank_bitboard(unsigned i) {
  return rank_file_to_bitboard(i, R1) | rank_file_to_bitboard(i, R2) |
         rank_file_to_bitboard(i, R3) | rank_file_to_bitboard(i, R4) |
         rank_file_to_bitboard(i, R5) | rank_file_to_bitboard(i, R6) |
         rank_file_to_bitboard(i, R7) | rank_file_to_bitboard(i, R8);
}

inline constexpr bitboard full_file_bitboard(unsigned i) {
  return rank_file_to_bitboard(FA, i) | rank_file_to_bitboard(FB, i) |
         rank_file_to_bitboard(FC, i) | rank_file_to_bitboard(FD, i) |
         rank_file_to_bitboard(FE, i) | rank_file_to_bitboard(FF, i) |
         rank_file_to_bitboard(FG, i) | rank_file_to_bitboard(FH, i);
}

constexpr auto const second_rank =
    std::array<bitboard, 2>{full_rank_bitboard(R2), full_rank_bitboard(R7)};

template <typename Fn>
void for_each_possible_move(position const& p, Fn&& f) {
  auto const* const moving_player =
      &p.piece_states_[p.to_move_ == color::WHITE ? 0 : NUM_PIECE_TYPES];
  auto const* const opposing_player =
      &p.piece_states_[p.to_move_ == color::BLACK ? 0 : NUM_PIECE_TYPES];

  auto const own_pieces = moving_player[PAWN] | moving_player[KNIGHT] |
                          moving_player[BISHOP] | moving_player[ROOK] |
                          moving_player[QUEEN] | moving_player[KING];
  auto const opposing_pieces = opposing_player[PAWN] | opposing_player[KNIGHT] |
                               opposing_player[BISHOP] | opposing_player[ROOK] |
                               opposing_player[QUEEN] | opposing_player[KING];

  auto pawns = moving_player[PAWN];
  auto second_rank_pawns = second_rank[p.to_move_] & pawns;
  while (pawns != 0U) {
    auto const pawn = bitboard{1U} << bitboard{cista::trailing_zeros(pawns)};
    pawns = second_rank_pawns & ~pawn;

    auto const occupied_squares = own_pieces | opposing_pieces;
    auto const single_jump_destination =
        p.to_move_ == color::WHITE ? pawn >> 8 : pawn << 8;
    if ((single_jump_destination & occupied_squares) == 0U) {
      f(move{pawn, single_jump_destination});
    }

    auto const double_jump_destination =
        p.to_move_ == color::WHITE ? pawn >> 16 : pawn << 16;
    if (((pawn & second_rank[p.to_move_]) != 0U) &&
        (single_jump_destination & occupied_squares) == 0U &&
        (double_jump_destination & occupied_squares) == 0U) {
      f(move{pawn, double_jump_destination});
    }

    auto const right_capture =
        p.to_move_ == color::WHITE ? pawn >> 7 : pawn << 9;
    if (right_capture & (opposing_pieces | p.en_passant_) &
        ~full_file_bitboard(FA)) {
      f(move{pawn, right_capture});
    }

    auto const left_capture =
        p.to_move_ == color::WHITE ? pawn >> 9 : pawn << 7;
    if (left_capture & (opposing_pieces | p.en_passant_) &
        ~full_file_bitboard(FH)) {
      f(move{pawn, left_capture});
    }
  }
}

}  // namespace chessbot