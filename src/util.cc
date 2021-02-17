#include "chessbot/util.h"

#include "cista/bit_counting.h"

namespace chessbot {

std::string get_square_name(bitboard const b) {
  auto const id = cista::trailing_zeros(b);
  return {file_names[id % 8], rank_names[id / 8]};
}

bitboard rank_file_to_bitboard(rank r, file f) {
  return bitboard{1} << ((8 - r) * 8 + f);
}

}  // namespace chessbot