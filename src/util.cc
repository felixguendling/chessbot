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

uint8_t name_to_square(std::string_view square_name) {
  auto const file = square_name[0] - 'a';
  auto const rank = 8 - (square_name[1] - '0');
  return rank * 8 + file;
}

}  // namespace chessbot