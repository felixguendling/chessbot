#pragma once

#include <cinttypes>
#include <array>
#include <iosfwd>
#include <span>
#include <string_view>

#include <iostream>  // TODO REMOVE

#include "chessbot/constants.h"
#include "chessbot/move.h"
#include "chessbot/util.h"

namespace chessbot {

struct castling_rights {
  bool white_can_short_castle_ : 1;
  bool white_can_long_castle_ : 1;
  bool black_can_short_castle_ : 1;
  bool black_can_long_castle_ : 1;
};

struct state_info {
  explicit state_info(bitboard ep, move m, castling_rights cr, uint8_t hmc)
      : en_passant_{ep},
        last_move_{m},
        castling_rights_{cr},
        half_move_clock_{hmc} {}
  bitboard en_passant_;
  move last_move_;
  piece_type captured_piece_{piece_type::NUM_PIECE_TYPES};
  castling_rights castling_rights_;
  uint8_t half_move_clock_;
};

struct position {
  friend std::ostream& operator<<(std::ostream&, position const&);
  friend std::istream& operator>>(std::istream&, position&);

  void print() const;
  state_info make_move(move);
  void undo_move(state_info);
  std::string to_str() const;
  std::string to_fen() const;
  void validate() const;

  color opposing_color() const {
    return to_move_ == color::WHITE ? color::BLACK : color::WHITE;
  }

  bitboard pieces(color const c, piece_type const pt) const {
    return piece_states_[pt] & pieces_by_color_[c];
  }

  void toggle_pieces(piece_type const pt, color const c,
                     bitboard const toggle) {
    pieces_by_color_[c] ^= toggle;
    piece_states_[pt] ^= toggle;
  }

  bool can_short_castle(color const c) const {
    return c == color::WHITE ? castling_rights_.white_can_short_castle_
                             : castling_rights_.black_can_short_castle_;
  }

  bool can_long_castle(color const c) const {
    return c == color::WHITE ? castling_rights_.white_can_long_castle_
                             : castling_rights_.black_can_long_castle_;
  }

  std::array<bitboard, NUM_PIECE_TYPES> piece_states_{};
  std::array<bitboard, 2> pieces_by_color_{};
  bitboard en_passant_{0U};
  uint8_t half_move_clock_{0U};
  unsigned full_move_count_{0U};
  color to_move_{color::WHITE};

  castling_rights castling_rights_;
};

}  // namespace chessbot
