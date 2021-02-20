#include "chessbot/util.h"

#include "cista/bit_counting.h"

namespace chessbot {

std::string get_square_name(bitboard const b) {
  auto const id = cista::trailing_zeros(b);
  return {file_names[id % 8], rank_names[id / 8]};
}

std::string bitboard_to_str(bitboard const bb) {
  std::string str;
  for (auto rank = 0U; rank != 8; ++rank) {
    for (auto file = 0U; file != 8; ++file) {
      auto const square_bb = rank_file_to_bitboard(rank, file);
      str += (square_bb & bb) ? '1' : '0';
    }
    str += '\n';
  }
  return str;
}

}  // namespace chessbot