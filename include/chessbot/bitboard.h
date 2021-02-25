#pragma once

#include <cinttypes>

namespace chessbot {

using bitboard = uint64_t;

template <typename Fn>
void for_each_set_bit(bitboard bb, Fn&& f) {
  while (bb != 0U) {

#ifdef _M_X64
    unsigned long square_idx = 0;
    _BitScanForward64(&square_idx, bb);
#else
    auto const square_idx = __builtin_ctzll(bb);
#endif

    auto const set_bit = bitboard{1U} << square_idx;
    bb ^= set_bit;
    f(set_bit);
  }
}

}  // namespace chessbot