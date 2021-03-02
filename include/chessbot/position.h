#pragma once

#include <cassert>
#include <cinttypes>
#include <array>
#include <iosfwd>
#include <span>
#include <string_view>

#include "chessbot/constants.h"
#include "chessbot/move.h"
#include "chessbot/util.h"
#include "chessbot/zobrist.h"

namespace chessbot {

struct castling_rights {
  bool white_can_short_castle_ : 1;
  bool white_can_long_castle_ : 1;
  bool black_can_short_castle_ : 1;
  bool black_can_long_castle_ : 1;
};

struct state_info {
  state_info() = default;

  explicit state_info(bitboard ep, move m, castling_rights cr, uint8_t hmc,
                      zobrist_t const prev_hash,
                      state_info const* prev_state_info)
      : en_passant_{ep},
        last_move_{m},
        castling_rights_{cr},
        half_move_clock_{hmc},
        prev_hash_{prev_hash},
        prev_state_info_{prev_state_info} {}

  void print_moves() const;

  template <typename Fn>
  void for_each_pred(Fn&& f) const {
    auto curr_state = this;
    while (curr_state != nullptr) {
      f(*curr_state);
      curr_state = curr_state->prev_state_info_;
    }
  }

  bitboard en_passant_;
  move last_move_;
  piece_type captured_piece_{piece_type::NUM_PIECE_TYPES};
  castling_rights castling_rights_;
  uint8_t half_move_clock_{0U};
  zobrist_t prev_hash_{0U};
  state_info const* prev_state_info_{nullptr};
};

struct position {
  friend std::ostream& operator<<(std::ostream&, position const&);
  friend std::istream& operator>>(std::istream&, position&);
  static position from_fen(std::string const&);

  void print() const;
  state_info make_move(move, state_info const* prev_state);
  void update_blockers_and_pinners(move, bitboard en_passant);
  std::string to_str() const;
  std::string to_fen() const;
  void print_trace(state_info const*) const;
  void validate() const;

  color opposing_color() const {
    return to_move_ == color::WHITE ? color::BLACK : color::WHITE;
  }

  bitboard all_pieces() const {
    return pieces_by_color_[color::WHITE] | pieces_by_color_[color::BLACK];
  }

  bitboard pieces(color const c, piece_type const pt) const {
    return piece_states_[pt] & pieces_by_color_[c];
  }

  template <color Color, piece_type PieceType>
  bitboard pieces() const {
    return piece_states_[PieceType] & pieces_by_color_[Color];
  }

  void toggle_pieces(piece_type const pt, color const c,
                     bitboard const toggle) {
    pieces_by_color_[c] ^= toggle;
    piece_states_[pt] ^= toggle;

    auto const square_idx = cista::trailing_zeros(toggle);
    assert(toggle >> square_idx == 1U);
    hash_ ^= zobrist_color_hashes[square_idx][c];
    hash_ ^= zobrist_piece_hashes[square_idx][pt];
  }

  template <color Color>
  bool can_short_castle() const {
    if constexpr (Color == color::WHITE) {
      return castling_rights_.white_can_short_castle_;
    } else {
      return castling_rights_.black_can_short_castle_;
    }
  }

  template <color Color>
  bool can_long_castle() const {
    if constexpr (Color == color::WHITE) {
      return castling_rights_.white_can_long_castle_;
    } else {
      return castling_rights_.black_can_long_castle_;
    }
  }

  std::array<bitboard, NUM_PIECE_TYPES> piece_states_{};
  std::array<bitboard, 2> pieces_by_color_{};
  std::array<bitboard, 2> checkers_{};
  std::array<bitboard, 2> blockers_for_king_{};
  std::array<bitboard, 2> pinners_{};
  zobrist_t hash_{0U};
  bitboard en_passant_{0U};
  uint8_t half_move_clock_{0U};
  unsigned full_move_count_{0U};
  color to_move_{color::WHITE};
  castling_rights castling_rights_;
};

}  // namespace chessbot
