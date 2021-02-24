#pragma once

#include <cinttypes>
#include <iosfwd>
#include <string>
#include <tuple>

#include "cista/bit_counting.h"

#include "chessbot/constants.h"

namespace chessbot {

enum class promotion_piece_type : uint16_t { KNIGHT, BISHOP, ROOK, QUEEN };
enum class special_move : uint16_t { NONE, PROMOTION, CASTLE };

struct move {
  move() = default;

  explicit move(std::string const&);

  explicit move(bitboard from, bitboard to)
      : from_field_{static_cast<uint16_t>(cista::trailing_zeros(from))},
        to_field_{static_cast<uint16_t>(cista::trailing_zeros(to))},
        special_move_{special_move::NONE},
        promotion_piece_type_{promotion_piece_type::KNIGHT} {}

  explicit move(bitboard from, bitboard to, promotion_piece_type ppt)
      : move{from, to} {
    promotion_piece_type_ = ppt;
  }

  bitboard from() const { return rank_file_to_bitboard(0, from_field_); }
  bitboard to() const { return rank_file_to_bitboard(0, to_field_); }

  friend std::ostream& operator<<(std::ostream&, move);
  std::string to_str() const;

  friend bool operator==(move const a, move const b) {
    return std::tie(a.from_field_, a.to_field_, a.promotion_piece_type_,
                    a.special_move_) == std::tie(b.from_field_, b.to_field_,
                                                 b.promotion_piece_type_,
                                                 b.special_move_);
  }
  friend bool operator!=(move const a, move const b) { return !(a == b); }

  uint16_t from_field_ : 6;
  uint16_t to_field_ : 6;
  promotion_piece_type promotion_piece_type_ : 2;
  special_move special_move_ : 2;
};

}  // namespace chessbot