#pragma once

#include <cinttypes>

#include "cista/bit_counting.h"

#include "chessbot/constants.h"

namespace chessbot {

struct move {
  explicit move(bitboard from, bitboard to)
      : from_field_{static_cast<uint16_t>(cista::trailing_zeros(from))},
        to_field_{static_cast<uint16_t>(cista::trailing_zeros(to))},
        special_move_{special_move::NONE} {}

  bitboard from() const { return rank_file_to_bitboard(0, from_field_); }
  bitboard to() const { return rank_file_to_bitboard(0, to_field_); }

  uint16_t from_field_ : 6;
  uint16_t to_field_ : 6;
  enum class promotion_piece_type : uint16_t {
    KNIGHT,
    BISHOP,
    ROOK,
    QUEEN
  } promotion_piece_type_ : 2;
  enum class special_move : uint16_t {
    NONE,
    PROMOTION,
    CASTLE
  } special_move_ : 2;
};

}  // namespace chessbot