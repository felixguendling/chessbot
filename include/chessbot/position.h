#pragma once

#include <cinttypes>
#include <array>
#include <iosfwd>
#include <span>
#include <string_view>

#include "chessbot/constants.h"

namespace chessbot {

struct move;

struct position {
  friend std::ostream& operator<<(std::ostream&, position const&);
  friend std::istream& operator>>(std::istream&, position&);

  void print() const;
  position make_move(move const&) const;
  std::string to_str() const;
  std::string to_fen() const;

  color opposing_color() const {
    return to_move_ == color::WHITE ? color::BLACK : color::WHITE;
  }

  bitboard pieces(color const c, piece_type const pt) const {
    return piece_statess_[pt] & pieces_by_color_[c];
  }

  void toggle_pieces(piece_type const pt, color const c,
                     bitboard const toggle) {
    pieces_by_color_[c] ^= toggle;
    piece_statess_[pt] ^= toggle;
  }

  bool can_short_castle(color const c) const {
    return c == color::WHITE ? castling_rights_.white_can_short_castle_
                             : castling_rights_.black_can_short_castle_;
  }

  bool can_long_castle(color const c) const {
    return c == color::WHITE ? castling_rights_.white_can_long_castle_
                             : castling_rights_.black_can_long_castle_;
  }

  std::array<bitboard, NUM_PIECE_TYPES> piece_statess_{};
  std::array<bitboard, 2> pieces_by_color_{};
  bitboard en_passant_{0U};
  unsigned half_move_clock_{0U};
  unsigned full_move_count_{0U};
  color to_move_{color::WHITE};

  struct castling_rights {
    bool white_can_short_castle_ : 1;
    bool white_can_long_castle_ : 1;
    bool black_can_short_castle_ : 1;
    bool black_can_long_castle_ : 1;
  } castling_rights_;
};

}  // namespace chessbot
