#pragma once

#include <cmath>

#include "chessbot/position.h"

#include "cista/bit_counting.h"

namespace chessbot {

struct move {
  explicit move(bitboard from, bitboard to)
      : from_field_{static_cast<uint16_t>(cista::trailing_zeros(from))},
        to_field_{static_cast<uint16_t>(cista::trailing_zeros(to))},
        special_move_{special_move::NONE} {}

  bitboard from() const { return rank_file_to_bitboard(0, from_field_); }
  bitboard to() const { return rank_file_to_bitboard(0, to_field_); }

  uint16_t from_field_ : 6;
  uint16_t to_field_ : 6;
  enum class promotion_piece_type : uint16_t {
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN
  } promotion_piece_type_ : 2;
  enum class special_move : uint16_t {
    NONE,
    PROMOTION,
    CASTLE
  } special_move_ : 2;
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

inline bitboard north_west(bitboard const bb, int const north, int const west) {
  auto const shift = north * 8 + west;
  return (shift < 0) ? bb << (-shift) : bb >> shift;
}

inline bitboard safe_north_west(bitboard const bb, int const north,
                                int const west) {
  auto const result = north_west(bb, north, west);

  auto const prev_id = cista::trailing_zeros(bb);
  auto const prev_file = prev_id % 8;
  auto const prev_rank = prev_id / 8;

  auto const id = cista::trailing_zeros(result);
  auto const file = id % 8;
  auto const rank = id / 8;

  if (north > 0 && prev_rank <= rank || north < 0 && prev_rank >= rank ||
      west > 0 && prev_file <= file || west < 0 && prev_file >= file) {
    return 0U;
  } else {
    return result;
  }
}

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

  auto const move_pawn_with_promotion_check = [&](move m) {
    if (m.to() & (full_rank_bitboard(R1) | full_rank_bitboard(R8))) {
      m.special_move_ = move::special_move::PROMOTION;
      m.promotion_piece_type_ = move::promotion_piece_type::KNIGHT;
      f(m);
      m.promotion_piece_type_ = move::promotion_piece_type::ROOK;
      f(m);
      m.promotion_piece_type_ = move::promotion_piece_type::BISHOP;
      f(m);
      m.promotion_piece_type_ = move::promotion_piece_type::QUEEN;
      f(m);
    } else {
      f(m);
    }
  };

  auto pawns = moving_player[PAWN];
  while (pawns != 0U) {
    auto const pawn = bitboard{1U} << bitboard{cista::trailing_zeros(pawns)};
    pawns = pawns & ~pawn;

    auto const occupied_squares = own_pieces | opposing_pieces;
    auto const single_jump_destination =
        p.to_move_ == color::WHITE ? pawn >> 8 : pawn << 8;
    if ((single_jump_destination & occupied_squares) == 0U) {
      move_pawn_with_promotion_check(move{pawn, single_jump_destination});
    }

    auto const double_jump_destination = p.to_move_ == color::WHITE
                                             ? north_west(pawn, 2, 0)
                                             : north_west(pawn, -2, 0);
    if (((pawn & second_rank[p.to_move_]) != 0U) &&
        (single_jump_destination & occupied_squares) == 0U &&
        (double_jump_destination & occupied_squares) == 0U) {
      f(move{pawn, double_jump_destination});
    }

    auto const right_capture =
        p.to_move_ == color::WHITE ? pawn >> 7 : pawn << 9;
    if (right_capture & (opposing_pieces | p.en_passant_) &
        ~full_file_bitboard(FA)) {
      move_pawn_with_promotion_check(move{pawn, right_capture});
    }

    auto const left_capture =
        p.to_move_ == color::WHITE ? pawn >> 9 : pawn << 7;
    if (left_capture & (opposing_pieces | p.en_passant_) &
        ~full_file_bitboard(FH)) {
      move_pawn_with_promotion_check(move{pawn, left_capture});
    }
  }

  auto knights = moving_player[KNIGHT];
  while (knights != 0U) {
    auto const knight = bitboard{1U}
                        << bitboard{cista::trailing_zeros(knights)};
    knights = knights & ~knight;

    for (auto const target :
         {safe_north_west(knight, 2, 1), safe_north_west(knight, 2, -1),
          safe_north_west(knight, -2, 1), safe_north_west(knight, -2, -1),
          safe_north_west(knight, 1, -2), safe_north_west(knight, 1, 2),
          safe_north_west(knight, -1, 2), safe_north_west(knight, -1, -2)}) {
      if (target != 0U && (target & own_pieces) == 0U) {
        f(move{knight, target});
      }
    }
  }
}

}  // namespace chessbot