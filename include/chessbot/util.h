#pragma once

#include <string>

#include "chessbot/constants.h"

namespace chessbot {

std::string get_square_name(bitboard);
std::string bitboard_to_str(bitboard);

static uint64_t x = 123456789, y = 362436069, z = 521288629;
inline uint64_t get_random_number() {
  unsigned long t;

  x ^= x << 16;
  x ^= x >> 5;
  x ^= x << 1;

  t = x;
  x = y;
  y = z;
  z = t ^ x ^ y;

  return z;
}

}  // namespace chessbot