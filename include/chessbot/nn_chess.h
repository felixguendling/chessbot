#pragma once

#include <algorithm>
#include <array>
#include <iomanip>
#include <iostream>
#include <map>

#include "chessbot/bitboard.h"
#include "chessbot/nn.h"
#include "chessbot/position.h"
#include "chessbot/stockfish_evals.h"

namespace chessbot {

constexpr auto const input_size = 6 * 64  // own pieces
                                  + 6 * 64;  // opponent pieces;
constexpr auto const output_size = 64  // from square
                                   * 64;  // to square

constexpr auto const illegal = real_t{-.1};
constexpr auto const printed_errors = 15;

inline std::array<real_t, input_size> nn_input_from_position(
    position const& p) {
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

inline std::array<real_t, output_size> to_expected(
    std::map<std::string, move_eval> const& evals) {
  auto expected = std::array<real_t, output_size>{};
  for (auto const& [move, eval] : evals) {
    auto const from = name_to_square(move.substr(0, 2));
    auto const to = name_to_square(move.substr(2, 2));
    expected[from + 64 * to] = sigmoid(eval.cp_ / 200.0);
  }
  return expected;
}

template <typename NN, size_t TestSetSize>
void print_move_evals_sorted_by_error(
    unsigned const n, NN const& nn,
    std::array<std::string, TestSetSize> const& fens,
    std::array<std::array<real_t, output_size>, TestSetSize> const& expected) {
  auto top_moves = std::array<uint32_t, output_size * TestSetSize>();
  //  std::generate(begin(top_moves), end(top_moves),
  //                [i = 0]() mutable { return i++; });
  auto i = 0;
  for (auto& m : top_moves) {
    m = i++;
  }
  auto nn_out = std::array<std::array<real_t, output_size>, TestSetSize>{};
  for (auto const& [i, fen] : utl::enumerate(fens)) {
    auto const p = position::from_fen(fen);
    nn_out[i] = nn.estimate(nn_input_from_position(p));
  }
  auto const sort_moves_by_error = [&](auto const it) {
    std::sort(begin(top_moves), it, [&](uint32_t const a, uint32_t const b) {
      auto const test_instance_idx_a = a / output_size;
      auto const move_idx_a = a % output_size;
      auto const test_instance_idx_b = b / output_size;
      auto const move_idx_b = b % output_size;
      return std::abs(expected[test_instance_idx_a][move_idx_a] -
                      nn_out[test_instance_idx_a][move_idx_a]) >
             std::abs(expected[test_instance_idx_b][move_idx_b] -
                      nn_out[test_instance_idx_b][move_idx_b]);
    });
  };
  auto const print = [&](auto const stop) {
    std::cout << "id     " << std::left << std::setw(82) << "fen  "
              << "move  " << std::setw(12) << "error  "
              << "stockfish  "
              << "nn"
              << "\n";
    for (auto it = begin(top_moves); it != stop; ++it) {
      auto test_instance_idx = *it / output_size;
      auto move_idx = *it % output_size;
      auto from = move_idx % 64;
      auto to = move_idx / 64;
      std::cout << std::left << std::setw(7) << *it << std::left
                << std::setw(80) << fens[test_instance_idx] << "  "
                << (get_square_name(bitboard{1} << from))
                << (get_square_name(bitboard{1} << to)) << "  " << std::setw(10)
                << std::abs(expected[test_instance_idx][move_idx] -
                            nn_out[test_instance_idx][move_idx])
                << "  " << std::setw(9) << expected[test_instance_idx][move_idx]
                << "  " << nn_out[test_instance_idx][move_idx] << "\n";
    }
  };
  sort_moves_by_error(end(top_moves));
  std::cout << n << " highest absolute errors:\n";
  print(begin(top_moves) + n);

  std::cout << "\n\nfalsely legal moves\n";
  auto it = std::partition(begin(top_moves), end(top_moves), [&](auto const a) {
    auto test_instance_idx = a / output_size;
    auto move_idx = a % output_size;
    return nn_out[test_instance_idx][move_idx] >= 0 &&
           expected[test_instance_idx][move_idx] == illegal;
  });
  sort_moves_by_error(it);
  print(it);

  std::cout << "\n\nfalsely illegal moves\n";
  it = std::partition(begin(top_moves), end(top_moves), [&](auto const a) {
    auto test_instance_idx = a / output_size;
    auto move_idx = a % output_size;
    return nn_out[test_instance_idx][move_idx] < 0 &&
           expected[test_instance_idx][move_idx] != illegal;
  });
  sort_moves_by_error(it);
  print(it);
}

template <typename Input, typename Expected, typename Network>
inline std::pair<real_t, real_t> determine_error(Network const& n,
                                                 Input const& input,
                                                 Expected const& expected) {
  auto error = std::pair<real_t, real_t>{};
  auto& [e1, e2] = error;
  for (auto batch_idx = 0U; batch_idx != input.size(); ++batch_idx) {
    auto const output = n.estimate(input[batch_idx]);
    for (auto output_idx = 0U; output_idx != output.size(); ++output_idx) {
      auto const diff =
          std::abs(output[output_idx] - expected[batch_idx][output_idx]);
      e1 += diff * diff;
      e2 = std::max(e2, diff);
    }
  }
  return error;
}

}  // namespace chessbot
