#pragma once

#include <cmath>
#include <cstdio>

#include "cista/bit_counting.h"

#include "utl/enumerate.h"

#include "chessbot/magic.h"
#include "chessbot/position.h"

namespace chessbot {

template <color ToMove>
inline bool is_valid_move(position const& p, move const m) {
  if (m.special_move_ == special_move::CASTLE) {
    return true;
  }

  auto const king_bb = p.pieces<ToMove, piece_type::KING>();
  auto const from = m.from();
  auto const to = m.to();
  auto const king = (m.from() & king_bb) ? to : king_bb;

  auto const pawn_attacks = [&]() {
    auto const opposing_pawns =
        p.pieces<opposing_color<ToMove>(), piece_type::PAWN>() & ~to;
    auto attacked_bb = bitboard{};
    if (ToMove == color::WHITE) {
      attacked_bb |= (opposing_pawns & ~full_file_bitboard(FA)) << 7;
      attacked_bb |= (opposing_pawns & ~full_file_bitboard(FH)) << 9;
    } else {
      attacked_bb |= (opposing_pawns & ~full_file_bitboard(FA)) >> 9;
      attacked_bb |= (opposing_pawns & ~full_file_bitboard(FH)) >> 7;
    }
    return attacked_bb;
  };

  auto const king_square_idx = cista::trailing_zeros(king);
  if (king && ((p.pieces<opposing_color<ToMove>(), piece_type::KNIGHT>() & ~to &
                knight_attacks_by_origin_square[king_square_idx]) ||
               (p.pieces<opposing_color<ToMove>(), piece_type::KING>() &
                king_attacks_by_origin_square[king_square_idx]) ||
               (pawn_attacks() & king))) {
    return false;
  }

  auto occupancy_after_move_bb = (p.all_pieces() & ~from) | to;
  if (p.en_passant_ == to && from & p.pieces<ToMove, piece_type::PAWN>()) {
    occupancy_after_move_bb &=
        ~north_west(to, ToMove == color::WHITE ? -1 : 1, 0);
  }

  auto const opposing_queen =
      p.pieces<opposing_color<ToMove>(), piece_type::QUEEN>();
  auto const opposing_streight =
      p.pieces<opposing_color<ToMove>(), piece_type::ROOK>() | opposing_queen;
  auto const opposing_diagonal =
      p.pieces<opposing_color<ToMove>(), piece_type::BISHOP>() | opposing_queen;
  if (king && (opposing_diagonal != 0U &&
                   (get_attack_squares<BISHOP>(king, occupancy_after_move_bb) &
                    opposing_diagonal & ~to) ||
               (opposing_streight != 0U &&
                (get_attack_squares<ROOK>(king, occupancy_after_move_bb) &
                 opposing_streight & ~to)))) {
    return false;
  }

  return true;
}

inline unsigned count_repetitions(position const& p,
                                  state_info const* const info) {
  auto repetitions = 0U;
  auto half_moves = p.half_move_clock_;
  auto curr_state = info;
  while (curr_state != nullptr && half_moves != 0U) {
    if (curr_state->prev_hash_ == p.hash_) {
      ++repetitions;
    }
    curr_state = curr_state->prev_state_info_;
    --half_moves;
  }
  return repetitions;
}

template <color ToMove, typename Fn>
bool generate_moves(position const& p, Fn&& f) {
  auto const own_pieces = p.pieces_by_color_[ToMove];
  auto const opposing_pieces = p.pieces_by_color_[opposing_color<ToMove>()];
  auto const all_pieces = own_pieces | opposing_pieces;
  auto f_called = false;

  auto const call_f_for_valid_move = [&](move const m) {
    if (is_valid_move<ToMove>(p, m)) {
      f_called = true;
      f(m);
    }
  };

  auto const move_pawn_with_promotion_check = [&](move m) {
    if (m.to() & (full_rank_bitboard(R1) | full_rank_bitboard(R8))) {
      m.special_move_ = special_move::PROMOTION;
      m.promotion_piece_type_ = promotion_piece_type::KNIGHT;
      call_f_for_valid_move(m);
      m.promotion_piece_type_ = promotion_piece_type::ROOK;
      call_f_for_valid_move(m);
      m.promotion_piece_type_ = promotion_piece_type::BISHOP;
      call_f_for_valid_move(m);
      m.promotion_piece_type_ = promotion_piece_type::QUEEN;
      call_f_for_valid_move(m);
    } else {
      call_f_for_valid_move(m);
    }
  };

  for_each_set_bit(
      p.pieces<ToMove, piece_type::PAWN>(), [&](bitboard const pawn) {
        auto const single_jump_destination =
            ToMove == color::WHITE ? pawn >> 8 : pawn << 8;
        if ((single_jump_destination & all_pieces) == 0U) {
          move_pawn_with_promotion_check(move{pawn, single_jump_destination});
        }

        auto const double_jump_destination = ToMove == color::WHITE
                                                 ? north_west(pawn, 2, 0)
                                                 : north_west(pawn, -2, 0);
        if (((pawn & second_rank[ToMove]) != 0U) &&
            (single_jump_destination & all_pieces) == 0U &&
            (double_jump_destination & all_pieces) == 0U) {
          call_f_for_valid_move(move{pawn, double_jump_destination});
        }

        auto const right_capture =
            ToMove == color::WHITE ? pawn >> 7 : pawn << 9;
        if (right_capture & (opposing_pieces | p.en_passant_) &
            ~full_file_bitboard(FA)) {
          move_pawn_with_promotion_check(move{pawn, right_capture});
        }

        auto const left_capture =
            ToMove == color::WHITE ? pawn >> 9 : pawn << 7;
        if (left_capture & (opposing_pieces | p.en_passant_) &
            ~full_file_bitboard(FH)) {
          move_pawn_with_promotion_check(move{pawn, left_capture});
        }
      });

  for_each_set_bit(
      p.pieces<ToMove, piece_type::KNIGHT>(), [&](bitboard const knight) {
        for_each_set_bit(
            knight_attacks_by_origin_square[cista::trailing_zeros(knight)],
            [&](bitboard const target) {
              if ((target & own_pieces) == 0U) {
                call_f_for_valid_move(move{knight, target});
              }
            });
      });

  for_each_set_bit(
      p.pieces<ToMove, piece_type::BISHOP>(), [&](bitboard const bishop) {
        for_each_set_bit(
            get_attack_squares<piece_type::BISHOP>(bishop, all_pieces) &
                ~own_pieces,
            [&](bitboard const attack_square) {
              call_f_for_valid_move(move{bishop, attack_square});
            });
      });

  for_each_set_bit(
      p.pieces<ToMove, piece_type::ROOK>(), [&](bitboard const rook) {
        for_each_set_bit(
            get_attack_squares<piece_type::ROOK>(rook, all_pieces) &
                ~own_pieces,
            [&](bitboard const attack_square) {
              call_f_for_valid_move(move{rook, attack_square});
            });
      });

  for_each_set_bit(
      p.pieces<ToMove, piece_type::QUEEN>(), [&](bitboard const queen) {
        for_each_set_bit(
            (get_attack_squares<piece_type::ROOK>(queen, all_pieces) |
             get_attack_squares<piece_type::BISHOP>(queen, all_pieces)) &
                ~own_pieces,
            [&](bitboard const attack_square) {
              call_f_for_valid_move(move{queen, attack_square});
            });
      });

  auto const king = p.pieces<ToMove, piece_type::KING>();
  if (king) {
    for_each_set_bit(king_attacks_by_origin_square[cista::trailing_zeros(king)],
                     [&](bitboard const target) {
                       if ((target & own_pieces) == 0U) {
                         call_f_for_valid_move(move{king, target});
                       }
                     });
  }

  auto const can_short_castle = [&]() {
    if (!p.can_short_castle(ToMove)) {
      return false;
    }

    if ((short_castle_rook_traversal_squares[ToMove] & all_pieces) ||
        (short_castle_knight_attack_squares[ToMove] &
         p.pieces<opposing_color<ToMove>(), piece_type::KNIGHT>()) ||
        (short_castle_pawn_king_attack_squares[ToMove] &
         (p.pieces<opposing_color<ToMove>(), KING>() |
          p.pieces<opposing_color<ToMove>(), PAWN>()))) {
      return false;
    }

    for (auto const king_traversal_square :
         short_castle_king_traversal_squares[ToMove]) {
      if (get_attack_squares<ROOK>(king_traversal_square, all_pieces) &
              (p.pieces<opposing_color<ToMove>(), piece_type::ROOK>() |
               p.pieces<opposing_color<ToMove>(), piece_type::QUEEN>()) ||
          get_attack_squares<BISHOP>(king_traversal_square, all_pieces) &
              (p.pieces<opposing_color<ToMove>(), piece_type::BISHOP>() |
               p.pieces<opposing_color<ToMove>(), piece_type::QUEEN>())) {
        return false;
      }
    }

    return true;
  };

  auto const can_long_castle = [&]() {
    if (!p.can_long_castle(ToMove)) {
      return false;
    }

    if ((long_castle_rook_traversal_squares[ToMove] & all_pieces) ||
        (long_castle_knight_attack_squares[ToMove] &
         p.pieces<opposing_color<ToMove>(), piece_type::KNIGHT>()) ||
        (long_castle_pawn_king_attack_squares[ToMove] &
         (p.pieces<opposing_color<ToMove>(), KING>() |
          p.pieces<opposing_color<ToMove>(), PAWN>()))) {
      return false;
    }

    for (auto const king_traversal_square :
         long_castle_king_traversal_squares[ToMove]) {
      if (get_attack_squares<ROOK>(king_traversal_square, all_pieces) &
              (p.pieces<opposing_color<ToMove>(), piece_type::ROOK>() |
               p.pieces<opposing_color<ToMove>(), piece_type::QUEEN>()) ||
          get_attack_squares<BISHOP>(king_traversal_square, all_pieces) &
              (p.pieces<opposing_color<ToMove>(), piece_type::BISHOP>() |
               p.pieces<opposing_color<ToMove>(), piece_type::QUEEN>())) {
        return false;
      }
    }

    return true;
  };

  auto const active_player_first_rank = ToMove == color::WHITE ? R1 : R8;
  if (can_short_castle()) {
    auto m = move{rank_file_to_bitboard(active_player_first_rank, FE),
                  rank_file_to_bitboard(active_player_first_rank, FH)};
    m.special_move_ = special_move::CASTLE;
    call_f_for_valid_move(m);
  }

  if (can_long_castle()) {
    auto m = move{rank_file_to_bitboard(active_player_first_rank, FE),
                  rank_file_to_bitboard(active_player_first_rank, FA)};
    m.special_move_ = special_move::CASTLE;
    call_f_for_valid_move(m);
  }

  return f_called;
}

template <typename Fn>
bool generate_moves(position const& p, Fn&& f) {
  return p.to_move_ == color::WHITE
             ? generate_moves<color::WHITE>(p, std::forward<Fn>(f))
             : generate_moves<color::BLACK>(p, std::forward<Fn>(f));
}

}  // namespace chessbot