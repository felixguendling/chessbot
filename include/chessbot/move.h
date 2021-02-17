#pragma once

#include <cmath>
#include <bit>

#include "chessbot/position.h"

#include "cista/bit_counting.h"

#include "utl/enumerate.h"

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

inline bitboard north_west(bitboard const bb, int const north, int const west) {
  auto const shift = north * 8 + west;
  return (shift < 0) ? bb << (-shift) : bb >> shift;
}

inline bitboard safe_north_west(bitboard const bb, int const north,
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

static auto const bishop_attack_bbs = []() {
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

static auto const rook_attack_bbs = []() {
  auto attacks = std::array<bitboard, 64>{};
  for (auto i = 0; i < 64; ++i) {
    auto const i_bb = bitboard{1} << i;
    for (auto j = -8; j < 8; ++j) {
      attacks[i] |= safe_north_west(i_bb, j, 0);
    }
    for (auto j = -8; j < 8; ++j) {
      attacks[i] |= safe_north_west(i_bb, 0, j);
    }
    for (auto edge_bb : {full_rank_bitboard(R1), full_rank_bitboard(R8),
                         full_file_bitboard(FA), full_file_bitboard(FH)}) {
      if ((i_bb & edge_bb) == 0U) {
        attacks[i] &= ~edge_bb;
      }
    }
    attacks[i] &= ~i_bb;
  }
  return attacks;
}();

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

constexpr auto const magic_number_num_bits = 14U;

inline std::array<int8_t, magic_number_num_bits> get_set_bit_indices(
    bitboard bb) {
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

inline bitboard guess_magic_number(unsigned square_idx, piece_type const pt) {
  while (true) {
    auto const magic_number = get_random_number();
    auto hits = std::array<bool, 1 << magic_number_num_bits>{};
    auto error = false;
    for_all_permutations(pt == ROOK ? rook_attack_bbs[square_idx]
                                    : bishop_attack_bbs[square_idx],
                         [&](bitboard const occupancy_permutation) -> bool {
                           auto const index =
                               (occupancy_permutation * magic_number) >>
                               (64U - magic_number_num_bits);
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

inline void attack_direction(bitboard& attack, bitboard const square,
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

inline bitboard rook_attack_squares(bitboard const square,
                                    bitboard const occupancy) {
  auto attack = bitboard{};
  attack_direction(attack, square, occupancy, 1, 0);
  attack_direction(attack, square, occupancy, -1, 0);
  attack_direction(attack, square, occupancy, 0, 1);
  attack_direction(attack, square, occupancy, 0, -1);
  return attack;
}

inline bitboard bishop_attack_squares(bitboard const square,
                                      bitboard const occupancy) {
  auto attack = bitboard{};
  attack_direction(attack, square, occupancy, 1, 1);
  attack_direction(attack, square, occupancy, -1, -1);
  attack_direction(attack, square, occupancy, 1, -1);
  attack_direction(attack, square, occupancy, -1, 1);
  return attack;
}

auto const bishop_square_magic_numbers = []() {
  auto square_magic_numbers = std::array<uint64_t, 64U>{};
  for (auto i = 0U; i < 64U; ++i) {
    square_magic_numbers[i] = guess_magic_number(i, piece_type::BISHOP);
  }
  return square_magic_numbers;
}();

auto const rook_square_magic_numbers = []() {
  auto square_magic_numbers = std::array<uint64_t, 64U>{};
  for (auto i = 0U; i < 64U; ++i) {
    square_magic_numbers[i] = guess_magic_number(i, piece_type::ROOK);
  }
  return square_magic_numbers;
}();

auto const magic_bishop_attack_squares = []() {
  std::array<std::array<bitboard, (1U << magic_number_num_bits)>, 64U>
      magic_attack_squares;
  for (auto i = 0U; i < 64; ++i) {
    for_all_permutations(
        bishop_attack_bbs[i], [&](bitboard const occupancy_permutation) {
          auto const permutation_index =
              (occupancy_permutation * bishop_square_magic_numbers[i]) >>
              (64 - magic_number_num_bits);
          magic_attack_squares[i][permutation_index] =
              bishop_attack_squares(bitboard{1} << i, occupancy_permutation);
          return false;
        });
  }
  return magic_attack_squares;
}();

auto const magic_rook_attack_squares = []() {
  std::array<std::array<bitboard, (1U << magic_number_num_bits)>, 64U>
      magic_attack_squares;
  for (auto i = 0U; i < 64; ++i) {
    for_all_permutations(
        rook_attack_bbs[i], [&](bitboard const occupancy_permutation) {
          auto const permutation_index =
              (occupancy_permutation * rook_square_magic_numbers[i]) >>
              (64 - magic_number_num_bits);
          magic_attack_squares[i][permutation_index] =
              rook_attack_squares(bitboard{1} << i, occupancy_permutation);
          return false;
        });
  }
  return magic_attack_squares;
}();

template <typename Fn>
void for_each_possible_move(position const& p, Fn&& f) {
  auto const* const moving_player =
      &p.piece_states_[p.to_move_ == color::WHITE ? 0 : NUM_PIECE_TYPES];
  auto const* const opposing_player =
      &p.piece_states_[p.to_move_ == color::BLACK ? 0 : NUM_PIECE_TYPES];

  auto const own_pieces = moving_player[PAWN] | moving_player[KNIGHT] |
                          moving_player[BISHOP] | moving_player[ROOK] |
                          moving_player[QUEEN] | moving_player[KING];
  auto const opposing_pieces = opposing_player[PAWN] | opposing_player[KNIGHT] |
                               opposing_player[BISHOP] | opposing_player[ROOK] |
                               opposing_player[QUEEN] | opposing_player[KING];

  auto const move_pawn_with_promotion_check = [&](move m) {
    if (m.to() & (full_rank_bitboard(R1) | full_rank_bitboard(R8))) {
      m.special_move_ = move::special_move::PROMOTION;
      m.promotion_piece_type_ = move::promotion_piece_type::KNIGHT;
      f(m);
      m.promotion_piece_type_ = move::promotion_piece_type::ROOK;
      f(m);
      m.promotion_piece_type_ = move::promotion_piece_type::BISHOP;
      f(m);
      m.promotion_piece_type_ = move::promotion_piece_type::QUEEN;
      f(m);
    } else {
      f(m);
    }
  };

  auto pawns = moving_player[PAWN];
  while (pawns != 0U) {
    auto const pawn = bitboard{1U} << bitboard{cista::trailing_zeros(pawns)};
    pawns = pawns & ~pawn;

    auto const occupied_squares = own_pieces | opposing_pieces;
    auto const single_jump_destination =
        p.to_move_ == color::WHITE ? pawn >> 8 : pawn << 8;
    if ((single_jump_destination & occupied_squares) == 0U) {
      move_pawn_with_promotion_check(move{pawn, single_jump_destination});
    }

    auto const double_jump_destination = p.to_move_ == color::WHITE
                                             ? north_west(pawn, 2, 0)
                                             : north_west(pawn, -2, 0);
    if (((pawn & second_rank[p.to_move_]) != 0U) &&
        (single_jump_destination & occupied_squares) == 0U &&
        (double_jump_destination & occupied_squares) == 0U) {
      f(move{pawn, double_jump_destination});
    }

    auto const right_capture =
        p.to_move_ == color::WHITE ? pawn >> 7 : pawn << 9;
    if (right_capture & (opposing_pieces | p.en_passant_) &
        ~full_file_bitboard(FA)) {
      move_pawn_with_promotion_check(move{pawn, right_capture});
    }

    auto const left_capture =
        p.to_move_ == color::WHITE ? pawn >> 9 : pawn << 7;
    if (left_capture & (opposing_pieces | p.en_passant_) &
        ~full_file_bitboard(FH)) {
      move_pawn_with_promotion_check(move{pawn, left_capture});
    }
  }

  auto knights = moving_player[KNIGHT];
  while (knights != 0U) {
    auto const knight = bitboard{1U}
                        << bitboard{cista::trailing_zeros(knights)};
    knights = knights & ~knight;

    for (auto const target :
         {safe_north_west(knight, 2, 1), safe_north_west(knight, 2, -1),
          safe_north_west(knight, -2, 1), safe_north_west(knight, -2, -1),
          safe_north_west(knight, 1, -2), safe_north_west(knight, 1, 2),
          safe_north_west(knight, -1, 2), safe_north_west(knight, -1, -2)}) {
      if (target != 0U && (target & own_pieces) == 0U) {
        f(move{knight, target});
      }
    }
  }

  auto bishops = moving_player[BISHOP];
  while (bishops != 0U) {
    auto const bishop = bitboard{1U}
                        << bitboard{cista::trailing_zeros(bishops)};
    bishops = bishops & ~bishop;
  }
}

}  // namespace chessbot