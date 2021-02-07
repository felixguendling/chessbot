#pragma once

#include <cinttypes>
#include <array>
#include <iosfwd>
#include <span>
#include <string_view>

#include "chessbot/constants.h"

namespace chessbot {

struct move;

inline constexpr bitboard rank_file_to_bitboard(unsigned rank, unsigned file) {
  return bitboard{1U} << bitboard{8U * rank + file};
}

struct position {
  friend std::ostream& operator<<(std::ostream&, position const&);
  friend std::istream& operator>>(std::istream&, position&);

  position make_move(move const&);
  std::string to_str() const;

  std::span<bitboard> get_pieces(color c) {
    return c == color::WHITE
               ? std::span<bitboard>{&piece_states_[0], NUM_PIECE_TYPES}
               : std::span<bitboard>{&piece_states_[NUM_PIECE_TYPES],
                                     NUM_PIECE_TYPES};
  }

  std::array<bitboard, NUM_PIECE_TYPES * 2> piece_states_{};
  bitboard en_passant_{0U};
  unsigned half_move_clock_{0U};
  unsigned full_move_count_{0U};
  color to_move_{color::WHITE};
  bool white_can_short_castle_{false};
  bool white_can_long_castle_{false};
  bool black_can_short_castle_{false};
  bool black_can_long_castle_{false};
};

}  // namespace chessbot
