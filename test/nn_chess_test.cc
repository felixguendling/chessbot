#include "doctest/doctest.h"

#include <iomanip>
#include <iostream>

#include "chessbot/generate_moves.h"
#include "chessbot/nn.h"
#include "chessbot/position.h"

using namespace chessbot;

constexpr auto const input_size = 6 * 64  // own pieces
                                  + 6 * 64;  // opponent pieces;
constexpr auto const output_size = 64  // from square
                                   * 64;  // to square

std::array<real_t, input_size> nn_input_from_position(position const& p) {
  auto input = std::array<real_t, input_size>{};
  auto offset = 0U;
  for (auto i = 0; i < NUM_PIECE_TYPES; ++i, offset += 64) {
    for_each_set_bit(p.piece_states_[i] & p.pieces_by_color_[p.to_move_],
                     [&](bitboard const bb) {
                       auto const idx = cista::trailing_zeros(bb);
                       input[offset + idx] = 1.0;
                     });
  }
  for (auto i = 0; i < NUM_PIECE_TYPES; ++i, offset += 64) {
    for_each_set_bit(
        p.piece_states_[i] & p.pieces_by_color_[p.opposing_color()],
        [&](bitboard const bb) {
          auto const idx = cista::trailing_zeros(bb);
          input[offset + idx] = 1.0;
        });
  }
  return input;
}

TEST_CASE("nn classifies legal moves - one position") {
  auto const fen = "4k3/8/8/8/4R3/4n3/r7/4K3 w - - 0 1";
  auto p = position::from_fen(fen);
  auto moves = std::array<move, max_moves>{};
  auto moves_begin = &moves[0];
  auto moves_end = generate_moves(p, moves_begin);

  auto n = std::make_unique<network<input_size, 32, output_size>>();

  auto const input = nn_input_from_position(p);

  auto expected = std::array<real_t, output_size>{};
  for (auto it = moves_begin; it != moves_end; ++it) {
    auto const from = it->from_field_;
    auto const to = it->to_field_;
    expected[from + 64 * to] = 1.0;
  }

  for (auto j = 0; j < 1000; ++j) {
    n->train(input, expected, 0.3);
  }

  auto const output = n->estimate(nn_input_from_position(p));
  for (auto i = 0U; i < 64 * 64; ++i) {
    const auto from_square_idx = i % 64;
    const auto to_square_idx = i / 64;
    if (std::any_of(moves_begin, moves_end,
                    [&from_square_idx, &to_square_idx](move const m) {
                      return m.from_field_ == from_square_idx &&
                             m.to_field_ == to_square_idx;
                    })) {
      CHECK(output[i] >= 0.99);
    } else {
      CHECK(output[i] <= 0.2);
    }
  }
}

TEST_CASE("nn classifies legal moves - random position" * doctest::skip(true)) {
  auto const fen = "4k3/8/8/8/8/4n3/r7/4K3 w - - 0 1";
  auto p = position::from_fen(fen);
  auto moves = std::array<move, max_moves>{};

  auto n = std::make_unique<network<input_size, 32, output_size>>();

  auto const generate_position = [&]() {
    auto copy = p;
    while (true) {
      auto const square_idx = get_random_number() % 64;
      auto const square_bb = bitboard{1} << square_idx;
      if ((copy.all_pieces() | full_rank_bitboard(R2)) & square_bb) {
        continue;
      }

      copy.pieces_by_color_[color::WHITE] ^= square_bb;
      copy.piece_states_[KNIGHT] ^= square_bb;
      return copy;
    }
  };

  for (auto j = 0; j < 10000; ++j) {
    auto const copy = generate_position();

    auto const input = nn_input_from_position(copy);

    auto moves_begin = &moves[0];
    auto moves_end = generate_moves(copy, moves_begin);

    auto expected = std::array<real_t, output_size>{};
    for (auto it = moves_begin; it != moves_end; ++it) {
      auto const from = it->from_field_;
      auto const to = it->to_field_;
      expected[from + 64 * to] = 1.0;
    }

    for (auto i = 0U; i < 500; ++i) {
      n->train(input, expected, 0.3);
    }
  }

  auto test_pos = position::from_fen("4k3/8/8/1N6/8/4n3/r7/4K3 w - - 0 1");
  auto const output = n->estimate(nn_input_from_position(test_pos));
  auto moves_begin = &moves[0];
  auto moves_end = generate_moves(test_pos, moves_begin);
  for (auto i = 0U; i < 64 * 64; ++i) {
    const auto from_square_idx = i % 64;
    const auto to_square_idx = i / 64;

    if (std::any_of(moves_begin, moves_end,
                    [&from_square_idx, &to_square_idx](move const m) {
                      return m.from_field_ == from_square_idx &&
                             m.to_field_ == to_square_idx;
                    })) {
      CHECK(output[i] >= 0.98);
    } else {
      CHECK(output[i] <= 0.2);
    }

    std::cout << get_square_name(bitboard{1} << from_square_idx) << " -> "
              << get_square_name(bitboard{1} << to_square_idx) << ": "
              << output[i] << "\n";
  }
}