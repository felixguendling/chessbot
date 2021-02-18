#include "chessbot/util.h"

#include "cista/bit_counting.h"

namespace chessbot {

std::string get_square_name(bitboard const b) {
  auto const id = cista::trailing_zeros(b);
  return {file_names[id % 8], rank_names[id / 8]};
}

}  // namespace chessbot