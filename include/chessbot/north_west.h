#pragma once

#include "cista/bit_counting.h"

#include "chessbot/bitboard.h"

namespace chessbot {

inline constexpr bitboard north_west(bitboard const bb, int const north,
                                     int const west) {
  auto const shift = north * 8 + west;
  if (shift >= 64 || shift <= -64) {
    return 0U;
  }
  return (shift < 0) ? bb << (-shift) : bb >> shift;
}

inline constexpr bitboard safe_north_west(bitboard const bb, int const north,
                                          int const west) {
  auto const result = north_west(bb, north, west);

  auto const prev_id = cista::trailing_zeros(bb);
  auto const prev_file = prev_id % 8;
  auto const prev_rank = prev_id / 8;

  auto const id = cista::trailing_zeros(result);
  auto const file = id % 8;
  auto const rank = id / 8;

  if (north > 0 && prev_rank <= rank || north < 0 && prev_rank >= rank ||
      west > 0 && prev_file <= file || west < 0 && prev_file >= file) {
    return 0U;
  } else {
    return result;
  }
}

}  // namespace chessbot