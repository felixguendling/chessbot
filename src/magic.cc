#include "chessbot/magic.h"

#include "chessbot/generate_moves.h"
#include "chessbot/util.h"

namespace chessbot {

std::array<bitboard, 64> bishop_attack_bbs = []() {
  auto attacks = std::array<bitboard, 64>{};
  for (auto i = 0; i < 64; ++i) {
    auto const i_bb = bitboard{1} << i;
    for (auto j = -8; j < 8; ++j) {
      attacks[i] |= safe_north_west(i_bb, j, j);
    }
    for (auto j = -8; j < 8; ++j) {
      attacks[i] |= safe_north_west(i_bb, j, -j);
    }
    attacks[i] &= ~edge_bitboard;
    attacks[i] &= ~i_bb;
  }
  return attacks;
}();

std::array<bitboard, 64> rook_attack_bbs = []() {
  auto attacks = std::array<bitboard, 64>{};
  for (auto square_idx = 0; square_idx < 64; ++square_idx) {
    auto const square_bb = bitboard{1} << square_idx;
    for (auto j = -8; j < 8; ++j) {
      attacks[square_idx] |= safe_north_west(square_bb, j, 0);
    }
    for (auto j = -8; j < 8; ++j) {
      attacks[square_idx] |= safe_north_west(square_bb, 0, j);
    }
    for (auto edge_bb : {full_rank_bitboard(R1), full_rank_bitboard(R8),
                         full_file_bitboard(FA), full_file_bitboard(FH)}) {
      if ((square_bb & edge_bb) == 0U) {
        attacks[square_idx] &= ~edge_bb;
      }
    }
    attacks[square_idx] &= ~square_bb;
  }
  return attacks;
}();

std::array<int8_t, magic_number_num_bits> get_set_bit_indices(bitboard bb) {
  auto set_bit_indices = std::array<int8_t, magic_number_num_bits>{};
  auto i = 0U;
  while (bb != 0U) {
    set_bit_indices[i] = cista::trailing_zeros(bb);
    bb &= ~(bitboard{1} << set_bit_indices[i]);
    ++i;
  }
  for (; i != set_bit_indices.size(); ++i) {
    set_bit_indices[i] = -1;
  }
  return set_bit_indices;
}

bitboard guess_magic_number(unsigned square_idx, piece_type const pt) {
  while (true) {
    auto const magic_number = get_random_number();
    auto hits = std::array<bool, 1 << magic_number_num_bits>{};
    auto error = false;
    for_all_permutations(pt == ROOK ? rook_attack_bbs[square_idx]
                                    : bishop_attack_bbs[square_idx],
                         [&](bitboard const occupancy_permutation) -> bool {
                           auto const index = get_magic_index(
                               occupancy_permutation, magic_number);
                           auto& entry = hits[index];
                           if (entry) {
                             error = true;
                             return true;
                           } else {
                             entry = true;
                             return false;
                           }
                         });
    if (!error) {
      return magic_number;
    }
  }
}

void attack_direction(bitboard& attack, bitboard const square,
                      bitboard const occupancy, int const north,
                      int const west) {
  for (auto i = 1; i != 8; ++i) {
    auto const attack_square = safe_north_west(square, north * i, west * i);
    if (attack_square == 0U) {
      break;
    }
    attack |= attack_square;
    if (attack_square & occupancy) {
      break;
    }
  }
};

bitboard rook_attack_squares(bitboard const square, bitboard const occupancy) {
  auto attack = bitboard{};
  attack_direction(attack, square, occupancy, 1, 0);
  attack_direction(attack, square, occupancy, -1, 0);
  attack_direction(attack, square, occupancy, 0, 1);
  attack_direction(attack, square, occupancy, 0, -1);
  return attack;
}

bitboard bishop_attack_squares(bitboard const square,
                               bitboard const occupancy) {
  auto attack = bitboard{};
  attack_direction(attack, square, occupancy, 1, 1);
  attack_direction(attack, square, occupancy, -1, -1);
  attack_direction(attack, square, occupancy, 1, -1);
  attack_direction(attack, square, occupancy, -1, 1);
  return attack;
}

std::array<uint64_t, 64U> bishop_square_magic_numbers = []() {
  auto square_magic_numbers = std::array<uint64_t, 64U>{};
  for (auto i = 0U; i < 64U; ++i) {
    square_magic_numbers[i] = guess_magic_number(i, piece_type::BISHOP);
  }
  return square_magic_numbers;
}();

std::array<uint64_t, 64U> rook_square_magic_numbers = []() {
  auto square_magic_numbers = std::array<uint64_t, 64U>{};
  for (auto i = 0U; i < 64U; ++i) {
    square_magic_numbers[i] = guess_magic_number(i, piece_type::ROOK);
  }
  return square_magic_numbers;
}();

std::array<std::array<bitboard, (1U << magic_number_num_bits)>, 64U>
    magic_bishop_attack_squares = []() {
      std::array<std::array<bitboard, (1U << magic_number_num_bits)>, 64U>
          magic_attack_squares;
      for (auto i = 0U; i < 64; ++i) {
        for_all_permutations(
            bishop_attack_bbs[i], [&](bitboard const occupancy_permutation) {
              auto const permutation_index = get_magic_index(
                  occupancy_permutation, bishop_square_magic_numbers[i]);
              magic_attack_squares[i][permutation_index] =
                  bishop_attack_squares(bitboard{1} << i,
                                        occupancy_permutation);
              return false;
            });
      }
      return magic_attack_squares;
    }();

std::array<std::array<bitboard, (1U << magic_number_num_bits)>, 64U>
    magic_rook_attack_squares = []() {
      std::array<std::array<bitboard, (1U << magic_number_num_bits)>, 64U>
          magic_attack_squares;
      for (auto i = 0U; i < 64; ++i) {
        for_all_permutations(
            rook_attack_bbs[i], [&](bitboard const occupancy_permutation) {
              auto const permutation_index = get_magic_index(
                  occupancy_permutation, rook_square_magic_numbers[i]);
              magic_attack_squares[i][permutation_index] =
                  rook_attack_squares(bitboard{1} << i, occupancy_permutation);
              return false;
            });
      }
      return magic_attack_squares;
    }();

}  // namespace chessbot