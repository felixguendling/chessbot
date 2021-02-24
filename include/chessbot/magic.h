#pragma once

#include <cinttypes>
#include <array>
#include <bit>

#include "cista/bit_counting.h"

#include "utl/enumerate.h"

#include "chessbot/position.h"

namespace chessbot {

constexpr auto const magic_number_num_bits = 14U;

extern std::array<bitboard, 64> bishop_attack_bbs;
extern std::array<bitboard, 64> rook_attack_bbs;

std::array<int8_t, magic_number_num_bits> get_set_bit_indices(bitboard);

template <typename Fn>
void for_all_permutations(bitboard const bb, Fn&& f) {
  auto const set_bit_indices = get_set_bit_indices(bb);
  auto const number_of_ones = std::popcount(bb);
  for (auto i = bitboard{0U}; i < (1U << number_of_ones); ++i) {
    auto occupancy_bb = bitboard{0U};
    for (auto const [j, set_bit_idx] : utl::enumerate(set_bit_indices)) {
      if (set_bit_idx == -1) {
        break;
      }
      occupancy_bb |= ((i >> j) & ~(full_bitboard - 1)) << set_bit_idx;
    }
    if (f(occupancy_bb)) {
      goto error;
    }
  }

error:
  return;
}

extern std::array<uint64_t, 64U> bishop_square_magic_numbers;

extern std::array<uint64_t, 64U> rook_square_magic_numbers;

extern std::array<std::array<bitboard, (1U << magic_number_num_bits)>, 64U>
    magic_bishop_attack_squares;

extern std::array<std::array<bitboard, (1U << magic_number_num_bits)>, 64U>
    magic_rook_attack_squares;

inline constexpr unsigned get_magic_index(bitboard const occupancy_permutation,
                                          uint64_t const magic_number) {
  return (occupancy_permutation * magic_number) >> (64 - magic_number_num_bits);
}

template <piece_type>
bitboard get_attack_squares(bitboard square, bitboard occupancy);

template <>
inline constexpr bitboard get_attack_squares<piece_type::ROOK>(
    bitboard const square, bitboard const occupancy) {
  auto const square_idx = cista::trailing_zeros(square);
  return magic_rook_attack_squares[square_idx][get_magic_index(
      rook_attack_bbs[square_idx] & occupancy,
      rook_square_magic_numbers[square_idx])];
}

template <>
inline constexpr bitboard get_attack_squares<piece_type::BISHOP>(
    bitboard const square, bitboard const occupancy) {
  auto const square_idx = cista::trailing_zeros(square);
  return magic_bishop_attack_squares[square_idx][get_magic_index(
      bishop_attack_bbs[square_idx] & occupancy,
      bishop_square_magic_numbers[square_idx])];
}

}  // namespace chessbot