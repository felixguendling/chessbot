#pragma once

#include <cinttypes>

#include "cista/bit_counting.h"

namespace chessbot {

using bitboard = uint64_t;

template <typename Fn>
void for_each_set_bit(bitboard bb, Fn&& f) {
  while (bb != 0U) {
    auto const square_idx = cista::trailing_zeros(bb);
    auto const set_bit = bitboard{1U} << bitboard{square_idx};
    bb = bb & ~set_bit;
    f(set_bit);
  }
}

}  // namespace chessbot