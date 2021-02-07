#pragma once

#include <cinttypes>
#include <array>
#include <iosfwd>
#include <string_view>

namespace chessbot {

enum class color : bool { WHITE, BLACK };
enum pieces { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NUM_PIECE_TYPES };
constexpr auto const white_pieces = std::string_view{"PNBRQK"};
constexpr auto const black_pieces = std::string_view{"pnbrqk"};
using piece_state = uint64_t;

inline piece_state rank_file_to_bitboard(unsigned rank, unsigned file) {
  return piece_state{1U} << piece_state{8U * rank + file};
}

struct position {
  friend std::ostream& operator<<(std::ostream&, position const&);
  friend std::istream& operator>>(std::istream&, position&);

  std::array<piece_state, NUM_PIECE_TYPES * 2> piece_states_{};
  piece_state en_passant_{0U};
  unsigned half_move_clock_{0U};
  unsigned full_move_count_{0U};
  color to_move_{color::WHITE};
  bool white_can_short_castle_{false};
  bool white_can_long_castle_{false};
  bool black_can_short_castle_{false};
  bool black_can_long_castle_{false};
};

}  // namespace chessbot
