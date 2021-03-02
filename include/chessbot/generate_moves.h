#pragma once

#include <cmath>
#include <cstdio>

#include "cista/bit_counting.h"

#include "utl/enumerate.h"

#include "chessbot/magic.h"
#include "chessbot/position.h"

namespace chessbot {

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

template <color ToMove>
move* generate_moves(position const& p, move* move_list) {
  auto const our_king = p.pieces<ToMove, piece_type::KING>();
  auto const opposing_pawns =
      p.pieces<opposing_color<ToMove>(), piece_type::PAWN>();
  const auto opposing_knights =
      p.pieces<opposing_color<ToMove>(), piece_type::KNIGHT>();
  const auto opposing_king =
      p.pieces<opposing_color<ToMove>(), piece_type::KING>();
  auto const opposing_queen =
      p.pieces<opposing_color<ToMove>(), piece_type::QUEEN>();
  auto const opposing_streight_movers =
      p.pieces<opposing_color<ToMove>(), piece_type::ROOK>() | opposing_queen;
  auto const opposing_diagonal_movers =
      p.pieces<opposing_color<ToMove>(), piece_type::BISHOP>() | opposing_queen;
  auto const king_square_idx = cista::trailing_zeros(p.pieces<ToMove, KING>());

  auto const compute_pawn_attacks = [&](bitboard const to) {
    auto opposing_pawns_without_captured = opposing_pawns & ~to;
    if (to & p.en_passant_) {
      opposing_pawns_without_captured &=
          ~(ToMove == color::WHITE ? to << 8 : to >> 8);
    }
    auto attacked_bb = bitboard{};
    if (ToMove == color::WHITE) {
      attacked_bb |= (opposing_pawns_without_captured & ~full_file_bitboard(FA))
                     << 7;
      attacked_bb |= (opposing_pawns_without_captured & ~full_file_bitboard(FH))
                     << 9;
    } else {
      attacked_bb |=
          (opposing_pawns_without_captured & ~full_file_bitboard(FA)) >> 9;
      attacked_bb |=
          (opposing_pawns_without_captured & ~full_file_bitboard(FH)) >> 7;
    }
    return attacked_bb;
  };
  auto const pawn_attacks_without_capture = compute_pawn_attacks(0U);
  auto const get_pawn_attacks = [&](bitboard const to) {
    return to & (opposing_pawns | p.en_passant_) ? compute_pawn_attacks(to)
                                                 : pawn_attacks_without_capture;
  };

  // OLD CODE
  auto const valid_move_old = [&](bitboard const from, bitboard const to,
                                  special_move const sm) {
    if (sm == special_move::CASTLE) {
      return true;
    }

    auto const king_bb = p.pieces<ToMove, piece_type::KING>();
    auto const king = (from & king_bb) ? to : king_bb;

    auto const king_square_idx = cista::trailing_zeros(king);
    if (king &&
        ((opposing_knights & ~to &
          knight_attacks_by_origin_square[king_square_idx]) ||
         (opposing_king & king_attacks_by_origin_square[king_square_idx]) ||
         (get_pawn_attacks(to) & king))) {
      return false;
    }

    auto occupancy_after_move_bb = (p.all_pieces() & ~from) | to;
    if (p.en_passant_ == to && from & p.pieces<ToMove, piece_type::PAWN>()) {
      occupancy_after_move_bb &=
          ~north_west(to, ToMove == color::WHITE ? -1 : 1, 0);
    }

    if (king && ((opposing_diagonal_movers != 0U &&
                  (get_attack_squares<BISHOP>(king, occupancy_after_move_bb) &
                   opposing_diagonal_movers & ~to)) ||
                 (opposing_streight_movers != 0U &&
                  (get_attack_squares<ROOK>(king, occupancy_after_move_bb) &
                   opposing_streight_movers & ~to)))) {
      return false;
    }

    return true;
  };

  // OLD CODE END

  auto const blocker_can_move = [&](bitboard const from, bitboard const to,
                                    bitboard const en_passant) {
    auto const from_idx = cista::trailing_zeros(from);
    auto const line_to_edge_bb =
        rook_line_to_edge_bb[king_square_idx][from_idx] |
        bishop_line_to_edge_bb[king_square_idx][from_idx];
    if (line_to_edge_bb & to) {
      return true;
    }
    auto valid = true;
    auto const updated_blockers_for_king =
        p.blockers_for_king_[ToMove] ^ (from | en_passant);
    for_each_set_bit(
        p.pinners_[ToMove] & line_to_edge_bb, [&](bitboard const pinner) {
          auto const pinner_idx = cista::trailing_zeros(pinner);
          auto const line_to_pinner =
              bishop_line_bb[king_square_idx][pinner_idx] |
              rook_line_bb[king_square_idx][pinner_idx];
          valid = valid && (line_to_pinner & updated_blockers_for_king);
        });
    return valid;
  };

  auto const is_valid_move = [&](bitboard const from, bitboard const to,
                                 special_move const sm) -> bool {
    if (sm == special_move::CASTLE ||
        (!p.checkers_[ToMove]  // no check
         && !(from & p.blockers_for_king_[ToMove])  // no blocker moved
         && !(from & our_king))  // not a king move
    ) {
      return true;
    } else if (from & our_king) {
      return valid_move_old(from, to, sm);
    } else if (p.checkers_[ToMove]) {  // in check
      if (std::popcount(p.checkers_[ToMove]) > 1) {  // multiple checkers
        return false;
      } else {  // in check from one piece
        // blocker for king moves
        if (from & p.blockers_for_king_[ToMove]) {
          if (!blocker_can_move(from, to, 0)) {
            return false;
          }
        }
        if (p.checkers_[ToMove] & to) {
          return true;
        }
        if ((p.en_passant_ & to) && (p.piece_states_[PAWN] & from)) {
          auto const capture_square_bb =
              ToMove == color::WHITE ? p.en_passant_ << 8 : p.en_passant_ >> 8;
          if (capture_square_bb & p.checkers_[ToMove]) {
            return true;
          }
        }
        if (p.checkers_[ToMove] &
            (p.piece_states_[ROOK] | p.piece_states_[QUEEN] |
             p.piece_states_[BISHOP])) {
          const auto checker = cista::trailing_zeros(p.checkers_[ToMove]);
          return (bishop_line_bb[king_square_idx][checker] |
                  rook_line_bb[king_square_idx][checker]) &
                 to;
        }
        return false;
      }
    } else if (from & p.blockers_for_king_[ToMove]) {
      if ((p.en_passant_ & to) && (p.piece_states_[PAWN] & from)) {
        auto const capture_square_bb =
            ToMove == color::WHITE ? p.en_passant_ << 8 : p.en_passant_ >> 8;
        if (!blocker_can_move(capture_square_bb, to, from)) {
          return false;
        }
      }
      return blocker_can_move(from, to, 0);
    }

    assert(false);
    return true;
  };

  auto const own_pieces = p.pieces_by_color_[ToMove];
  auto const opposing_pieces = p.pieces_by_color_[opposing_color<ToMove>()];
  auto const all_pieces = own_pieces | opposing_pieces;

  auto const call_f_for_valid_move =
      [&](bitboard const from, bitboard const to,
          special_move const sm = special_move::NONE,
          promotion_piece_type const promotion_piece =
              promotion_piece_type::BISHOP) {
        if (is_valid_move(from, to, sm)) {
          *move_list++ = move{from, to, sm, promotion_piece};
        }
      };

  auto const move_pawn_with_promotion_check =
      [&](bitboard const from, bitboard const to,
          special_move const type = special_move::NONE) {
        if (to & (full_rank_bitboard(R1) | full_rank_bitboard(R8))) {
          call_f_for_valid_move(from, to, special_move::PROMOTION,
                                promotion_piece_type::KNIGHT);
          call_f_for_valid_move(from, to, special_move::PROMOTION,
                                promotion_piece_type::ROOK);
          call_f_for_valid_move(from, to, special_move::PROMOTION,
                                promotion_piece_type::BISHOP);
          call_f_for_valid_move(from, to, special_move::PROMOTION,
                                promotion_piece_type::QUEEN);
        } else {
          call_f_for_valid_move(from, to);
        }
      };

  auto const pawns = p.pieces<ToMove, piece_type::PAWN>();
  auto const single_jump_pawn_destinations =
      (ToMove == color::WHITE ? pawns >> 8 : pawns << 8) & ~all_pieces;
  auto const single_jump_pawns = ToMove == color::WHITE
                                     ? single_jump_pawn_destinations << 8
                                     : single_jump_pawn_destinations >> 8;
  auto const double_jump_pawn_destinations =
      (ToMove == color::WHITE ? single_jump_pawns >> 16
                              : single_jump_pawns << 16) &
      ~all_pieces;
  auto const double_jump_pawns =
      second_rank[ToMove] &
      (ToMove == color::WHITE ? double_jump_pawn_destinations << 16
                              : double_jump_pawn_destinations >> 16);

  for_each_set_bit(single_jump_pawns, [&](bitboard const pawn) {
    move_pawn_with_promotion_check(
        pawn, ToMove == color::WHITE ? pawn >> 8 : pawn << 8);
  });

  for_each_set_bit(double_jump_pawns, [&](bitboard const pawn) {
    call_f_for_valid_move(pawn,
                          ToMove == color::WHITE ? pawn >> 16 : pawn << 16);
  });

  auto const pawn_right_capture_destinations =
      (ToMove == color::WHITE ? pawns >> 7 : pawns << 9) &
      (opposing_pieces | p.en_passant_) & ~full_file_bitboard(FA);
  auto const right_capture_pawns = ToMove == color::WHITE
                                       ? pawn_right_capture_destinations << 7
                                       : pawn_right_capture_destinations >> 9;
  for_each_set_bit(right_capture_pawns, [&](bitboard const pawn) {
    auto const right_capture = ToMove == color::WHITE ? pawn >> 7 : pawn << 9;
    move_pawn_with_promotion_check(pawn, right_capture);
  });

  auto const pawn_left_capture_destinations =
      (ToMove == color::WHITE ? pawns >> 9 : pawns << 7) &
      (opposing_pieces | p.en_passant_) & ~full_file_bitboard(FH);
  auto const left_capture_pawns = ToMove == color::WHITE
                                      ? pawn_left_capture_destinations << 9
                                      : pawn_left_capture_destinations >> 7;
  for_each_set_bit(left_capture_pawns, [&](bitboard const pawn) {
    auto const left_capture = ToMove == color::WHITE ? pawn >> 9 : pawn << 7;
    move_pawn_with_promotion_check(pawn, left_capture);
  });

  for_each_set_bit(
      p.pieces<ToMove, piece_type::KNIGHT>(), [&](bitboard const knight) {
        for_each_set_bit(
            knight_attacks_by_origin_square[cista::trailing_zeros(knight)],
            [&](bitboard const target) {
              if ((target & own_pieces) == 0U) {
                call_f_for_valid_move(knight, target);
              }
            });
      });

  for_each_set_bit(
      p.pieces<ToMove, piece_type::BISHOP>(), [&](bitboard const bishop) {
        for_each_set_bit(
            get_attack_squares<piece_type::BISHOP>(bishop, all_pieces) &
                ~own_pieces,
            [&](bitboard const attack_square) {
              call_f_for_valid_move(bishop, attack_square);
            });
      });

  for_each_set_bit(
      p.pieces<ToMove, piece_type::ROOK>(), [&](bitboard const rook) {
        for_each_set_bit(
            get_attack_squares<piece_type::ROOK>(rook, all_pieces) &
                ~own_pieces,
            [&](bitboard const attack_square) {
              call_f_for_valid_move(rook, attack_square);
            });
      });

  for_each_set_bit(
      p.pieces<ToMove, piece_type::QUEEN>(), [&](bitboard const queen) {
        for_each_set_bit(
            (get_attack_squares<piece_type::ROOK>(queen, all_pieces) |
             get_attack_squares<piece_type::BISHOP>(queen, all_pieces)) &
                ~own_pieces,
            [&](bitboard const attack_square) {
              call_f_for_valid_move(queen, attack_square);
            });
      });

  auto const king = p.pieces<ToMove, piece_type::KING>();
  if (king) {
    for_each_set_bit(king_attacks_by_origin_square[cista::trailing_zeros(king)],
                     [&](bitboard const target) {
                       if ((target & own_pieces) == 0U) {
                         call_f_for_valid_move(king, target);
                       }
                     });
  }

  auto const can_short_castle = [&]() {
    if (!p.can_short_castle<ToMove>()) {
      return false;
    }

    if ((short_castle_rook_traversal_squares[ToMove] & all_pieces) ||
        (short_castle_knight_attack_squares[ToMove] & opposing_knights) ||
        (short_castle_pawn_king_attack_squares[ToMove] &
         (opposing_king | opposing_pawns))) {
      return false;
    }

    for (auto const king_traversal_square :
         short_castle_king_traversal_squares[ToMove]) {
      if (get_attack_squares<ROOK>(king_traversal_square, all_pieces) &
              opposing_streight_movers ||
          get_attack_squares<BISHOP>(king_traversal_square, all_pieces) &
              opposing_diagonal_movers) {
        return false;
      }
    }

    return true;
  };

  auto const can_long_castle = [&]() {
    if (!p.can_long_castle<ToMove>()) {
      return false;
    }

    if ((long_castle_rook_traversal_squares[ToMove] & all_pieces) ||
        (long_castle_knight_attack_squares[ToMove] & opposing_knights) ||
        (long_castle_pawn_king_attack_squares[ToMove] &
         (opposing_king | opposing_pawns))) {
      return false;
    }

    for (auto const king_traversal_square :
         long_castle_king_traversal_squares[ToMove]) {
      if ((get_attack_squares<ROOK>(king_traversal_square, all_pieces) &
           opposing_streight_movers) ||
          (get_attack_squares<BISHOP>(king_traversal_square, all_pieces) &
           opposing_diagonal_movers)) {
        return false;
      }
    }

    return true;
  };

  auto const active_player_first_rank = ToMove == color::WHITE ? R1 : R8;
  if (can_short_castle()) {
    call_f_for_valid_move(rank_file_to_bitboard(active_player_first_rank, FE),
                          rank_file_to_bitboard(active_player_first_rank, FH),
                          special_move::CASTLE);
  }

  if (can_long_castle()) {
    call_f_for_valid_move(rank_file_to_bitboard(active_player_first_rank, FE),
                          rank_file_to_bitboard(active_player_first_rank, FA),
                          special_move::CASTLE);
  }

  return move_list;
}

inline move* generate_moves(position const& p, move* move_list) {
  return p.to_move_ == color::WHITE
             ? generate_moves<color::WHITE>(p, move_list)
             : generate_moves<color::BLACK>(p, move_list);
}

}  // namespace chessbot