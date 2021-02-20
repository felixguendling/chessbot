#pragma once

#include <cmath>
#include <cstdio>

#include "cista/bit_counting.h"

#include "utl/enumerate.h"

#include "chessbot/magic.h"
#include "chessbot/position.h"

namespace chessbot {

inline bitboard north_west(bitboard const bb, int const north, int const west) {
  auto const shift = north * 8 + west;
  if (shift >= 64 || shift <= -64) {
    return 0U;
  }
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
void for_each_set_bit(bitboard bb, Fn&& f) {
  while (bb != 0U) {
    auto const square_idx = cista::trailing_zeros(bb);
    auto const set_bit = bitboard{1U} << bitboard{square_idx};
    bb = bb & ~set_bit;
    f(set_bit);
  }
}

template <typename Fn>
void for_each_possible_move(position const& p, Fn&& f) {
  auto const own_pieces = p.pieces_by_color_[p.to_move_];
  auto const opposing_pieces = p.pieces_by_color_[p.opposing_color()];
  auto const all_pieces = own_pieces | opposing_pieces;

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

  for_each_set_bit(
      p.pieces(p.to_move_, piece_type::PAWN), [&](bitboard const pawn) {
        auto const single_jump_destination =
            p.to_move_ == color::WHITE ? pawn >> 8 : pawn << 8;
        if ((single_jump_destination & all_pieces) == 0U) {
          move_pawn_with_promotion_check(move{pawn, single_jump_destination});
        }

        auto const double_jump_destination = p.to_move_ == color::WHITE
                                                 ? north_west(pawn, 2, 0)
                                                 : north_west(pawn, -2, 0);
        if (((pawn & second_rank[p.to_move_]) != 0U) &&
            (single_jump_destination & all_pieces) == 0U &&
            (double_jump_destination & all_pieces) == 0U) {
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
      });

  for_each_set_bit(
      p.pieces(p.to_move_, piece_type::KNIGHT), [&](bitboard const knight) {
        for (auto const target :
             {safe_north_west(knight, 2, 1), safe_north_west(knight, 2, -1),
              safe_north_west(knight, -2, 1), safe_north_west(knight, -2, -1),
              safe_north_west(knight, 1, -2), safe_north_west(knight, 1, 2),
              safe_north_west(knight, -1, 2),
              safe_north_west(knight, -1, -2)}) {
          if (target != 0U && (target & own_pieces) == 0U) {
            f(move{knight, target});
          }
        }
      });

  for_each_set_bit(
      p.pieces(p.to_move_, piece_type::BISHOP), [&](bitboard const bishop) {
        for_each_set_bit(
            get_attack_squares<piece_type::BISHOP>(bishop, all_pieces) &
                ~own_pieces,
            [&](bitboard const attack_square) {
              f(move{bishop, attack_square});
            });
      });

  for_each_set_bit(
      p.pieces(p.to_move_, piece_type::ROOK), [&](bitboard const rook) {
        for_each_set_bit(
            get_attack_squares<piece_type::ROOK>(rook, all_pieces) &
                ~own_pieces,
            [&](bitboard const attack_square) {
              f(move{rook, attack_square});
            });
      });

  for_each_set_bit(
      p.pieces(p.to_move_, piece_type::QUEEN), [&](bitboard const queen) {
        for_each_set_bit(
            (get_attack_squares<piece_type::ROOK>(queen, all_pieces) |
             get_attack_squares<piece_type::BISHOP>(queen, all_pieces)) &
                ~own_pieces,
            [&](bitboard const attack_square) {
              f(move{queen, attack_square});
            });
      });

  auto const king = p.pieces(p.to_move_, piece_type::KING);
  for (auto const target :
       {safe_north_west(king, 1, 0), safe_north_west(king, 1, -1),
        safe_north_west(king, 1, 1), safe_north_west(king, 0, -1),
        safe_north_west(king, 0, 1), safe_north_west(king, -1, 1),
        safe_north_west(king, -1, 0), safe_north_west(king, -1, -1)}) {
    if (target != 0U && (target & own_pieces) == 0U) {
      f(move{king, target});
    }
  }

  auto const can_short_castle = [&]() {
    if (!p.can_short_castle(p.to_move_)) {
      return false;
    }

    if ((short_castle_rook_traversal_squares[p.to_move_] & all_pieces) ||
        (short_castle_knight_attack_squares[p.to_move_] &
         p.pieces(p.opposing_color(), piece_type::KNIGHT)) ||
        (short_castle_pawn_king_attack_squares[p.to_move_] &
         (p.pieces(p.opposing_color(), KING) |
          p.pieces(p.opposing_color(), PAWN)))) {
      return false;
    }

    for (auto const king_traversal_square :
         short_castle_king_traversal_squares[p.to_move_]) {
      if (get_attack_squares<ROOK>(king_traversal_square, all_pieces) &
              (p.pieces(p.opposing_color(), piece_type::ROOK) |
               p.pieces(p.opposing_color(), piece_type::QUEEN)) ||
          get_attack_squares<BISHOP>(king_traversal_square, all_pieces) &
              (p.pieces(p.opposing_color(), piece_type::BISHOP) |
               p.pieces(p.opposing_color(), piece_type::QUEEN))) {
        return false;
      }
    }

    return true;
  };

  auto const can_long_castle = [&]() {
    if (!p.can_long_castle(p.to_move_)) {
      return false;
    }

    if ((long_castle_rook_traversal_squares[p.to_move_] & all_pieces) ||
        (long_castle_knight_attack_squares[p.to_move_] &
         p.pieces(p.opposing_color(), piece_type::KNIGHT)) ||
        (long_castle_pawn_king_attack_squares[p.to_move_] &
         (p.pieces(p.opposing_color(), KING) |
          p.pieces(p.opposing_color(), PAWN)))) {
      return false;
    }

    for (auto const king_traversal_square :
         long_castle_king_traversal_squares[p.to_move_]) {
      if (get_attack_squares<ROOK>(king_traversal_square, all_pieces) &
              (p.pieces(p.opposing_color(), piece_type::ROOK) |
               p.pieces(p.opposing_color(), piece_type::QUEEN)) ||
          get_attack_squares<BISHOP>(king_traversal_square, all_pieces) &
              (p.pieces(p.opposing_color(), piece_type::BISHOP) |
               p.pieces(p.opposing_color(), piece_type::QUEEN))) {
        return false;
      }
    }

    return true;
  };

  auto const active_player_first_rank = p.to_move_ == color::WHITE ? R1 : R8;
  if (can_short_castle()) {
    auto m = move{rank_file_to_bitboard(active_player_first_rank, FE),
                  rank_file_to_bitboard(active_player_first_rank, FH)};
    m.special_move_ = move::special_move::CASTLE;
    f(m);
  }

  if (can_long_castle()) {
    auto m = move{rank_file_to_bitboard(active_player_first_rank, FE),
                  rank_file_to_bitboard(active_player_first_rank, FA)};
    m.special_move_ = move::special_move::CASTLE;
    f(m);
  }
}
}  // namespace chessbot