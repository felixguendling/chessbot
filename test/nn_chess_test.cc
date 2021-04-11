#include "doctest/doctest.h"

#include <iomanip>
#include <iostream>

#include "chessbot/generate_moves.h"
#include "chessbot/nn.h"
#include "chessbot/pgn.h"
#include "chessbot/position.h"
#include "chessbot/stockfish_evals.h"

#include "plot.h"

using namespace chessbot;

constexpr auto const input_size = 6 * 64  // own pieces
                                  + 6 * 64;  // opponent pieces;
constexpr auto const output_size = 64  // from square
                                   * 64;  // to square

constexpr auto const illegal = real_t{-.1};
constexpr auto const printed_errors = 15;

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
      CHECK(output[i] >= 0.95);
    } else {
      CHECK(output[i] <= 0.2);
    }
  }
}

TEST_CASE(
    "nn classifies legal moves - random position" /* * doctest::skip(true) */) {
  srand(0);

  auto pl_absolute_errors =
      plot{"legal moves from random position - absolute errors"};
  auto pl_relative_errors =
      plot{"legal moves from random position - relative errors"};

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
      copy.piece_states_[QUEEN] ^= square_bb;
      return copy;
    }
  };

  auto const determine_error = [&]() {
    auto absolute_error = 0U;
    auto relative_error = 0.0;
    auto count = 0U;
    for (auto square_idx = 0U; square_idx < 64; ++square_idx) {
      auto const square_bb = bitboard{1} << square_idx;
      if ((p.all_pieces() | full_rank_bitboard(R2)) & square_bb) {
        continue;
      }

      auto copy = p;
      copy.pieces_by_color_[color::WHITE] ^= square_bb;
      copy.piece_states_[QUEEN] ^= square_bb;

      auto const output = n->estimate(nn_input_from_position(copy));
      auto moves_begin = &moves[0];
      auto moves_end = generate_moves(copy, moves_begin);
      for (auto i = 0U; i < 64 * 64; ++i) {
        const auto from_square_idx = i % 64;
        const auto to_square_idx = i / 64;

        if (std::any_of(moves_begin, moves_end,
                        [&from_square_idx, &to_square_idx](move const m) {
                          return m.from_field_ == from_square_idx &&
                                 m.to_field_ == to_square_idx;
                        })) {
          absolute_error += output[i] >= 0.98 ? 0 : 1;
          relative_error += 1 - output[i];
        } else {
          absolute_error += output[i] <= 0.2 ? 0 : 1;
          relative_error += output[i];
        }

        ++count;
      }
    }

    return std::pair{absolute_error, relative_error / count};
  };

  auto learning_rate = 0.7;
  for (auto j = 0; j < 10000; ++j) {
    if (j == 2000) {
      learning_rate = 0.3;
    }
    if (j == 7000) {
      learning_rate = 0.1;
    }
    if (j == 8000) {
      learning_rate = 0.01;
    }

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
      n->train(input, expected, learning_rate);
    }

    if (j % 20 == 0) {
      auto const [absolute_error, relative_error] = determine_error();
      pl_absolute_errors.add_entry(j, absolute_error);
      pl_relative_errors.add_entry(j, relative_error);
    }

    std::cout << "iteration: " << j << "\n";
  }

  auto test_pos = position::from_fen("4k3/8/8/1Q6/8/4n3/r7/4K3 w - - 0 1");
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

  pl_absolute_errors.do_plot();
  pl_relative_errors.do_plot();
}

TEST_CASE("train from pgn") {
  auto const learning_rate = 0.53;
  auto const inner_loop_size = 1000;
  auto pgn = utl::cstr{
      R"([Event "Rated Classical tournament https://lichess.org/tournament/whc7Blcq"]
[Site "https://lichess.org/z0CDaIfk"]
[White "AJEDREZSUPERIOR"]
[Black "HighLevelUser"]
[Result "0-1"]
[UTCDate "2017.03.31"]
[UTCTime "22:06:32"]
[WhiteElo "1687"]
[BlackElo "1954"]
[WhiteRatingDiff "-4"]
[BlackRatingDiff "+5"]
[ECO "D04"]
[Opening "Queen's Pawn Game: Colle System"]
[TimeControl "600+10"]
[Termination "Normal"]

1. e4 e5 2. Bc4 Bc5 3. Nc3 Qf6 4. Nge2 Qxf2# 1-0)"};
  auto const g = parse_pgn(pgn);

  for (auto m : g.moves_) {
    std::cout << m << "\n";
  }

  auto n = std::make_unique<network<input_size, 32, output_size>>();
  auto p = position::from_fen(start_position_fen);

  auto const to_expected = [&](std::map<std::string, float> const& evals) {
    auto expected = std::array<real_t, output_size>{};
    for (auto const& [move, eval] : evals) {
      auto const from = name_to_square(move.substr(0, 2));
      auto const to = name_to_square(move.substr(2, 2));
      expected[from + 64 * to] = eval;
      std::cout << move << ": " << eval << "\n";
    }
    return expected;
  };

  constexpr auto const batch_size = 8;
  auto input = std::array<std::array<real_t, input_size>, batch_size>{};
  auto expected = std::array<std::array<real_t, output_size>, batch_size>{};
  auto batch_idx = 0U;
  for (auto const& m : g.moves_) {
    auto const sf_evals = stockfish_evals(p);

    input[batch_idx] = nn_input_from_position(p);
    expected[batch_idx] = to_expected(sf_evals);

    p.make_pgn_move(m, nullptr);
    std::cout << "\n\nmove made: " << m << "\n";
  }

  for (auto i = 0; i < inner_loop_size; i++) {
    n->train_epoch(input, expected, learning_rate);
  }

  auto const print_best_moves = [&]() {
    auto nn_evals = n->estimate(nn_input_from_position(p));
    auto best_moves = std::array<uint16_t, output_size>{};
    for (auto i = 0; i < output_size; ++i) {
      best_moves[i] = i;
    }
    std::sort(begin(best_moves), end(best_moves),
              [&](uint16_t const a, uint16_t const b) {
                return nn_evals[a] > nn_evals[b];
              });
    for (auto i = begin(best_moves); i != begin(best_moves) + 20; ++i) {
      auto from = *i % 64;
      auto to = *i / 64;
      auto e = nn_evals[*i];
      std::cout << (get_square_name(bitboard{1} << from))
                << (get_square_name(bitboard{1} << to)) << ": " << e << "\n";
    }
  };
  p = position::from_fen(start_position_fen);
  for (auto const& m : g.moves_) {
    print_best_moves();
    std::cout << "\nmove made: " << m << " -> BEST MOVES:\n";
    p.make_pgn_move(m, nullptr);
  }
}

std::array<real_t, output_size> to_expected(
    std::map<std::string, float> const& evals) {
  auto expected = std::array<real_t, output_size>{};
  std::fill(begin(expected), end(expected), illegal);
  for (auto const& [move, eval] : evals) {
    auto const from = name_to_square(move.substr(0, 2));
    auto const to = name_to_square(move.substr(2, 2));
    expected[from + 64 * to] = eval;
  }
  return expected;
}

template <size_t TestSetSize>
auto stockfish_evals_test_set(
    std::array<std::string, TestSetSize> const& fens) {
  auto expected = std::array<std::array<real_t, output_size>, TestSetSize>{};
  for (auto const& [i, fen] : utl::enumerate(fens)) {
    expected[i] = to_expected(stockfish_evals(position::from_fen(fen)));
  }
  return expected;
}

template <typename NN, size_t TestSetSize>
void print_move_evals_sorted_by_error(
    unsigned const n, NN const& nn,
    std::array<std::string, TestSetSize> const& fens,
    std::array<std::array<real_t, output_size>, TestSetSize> const& expected) {
  auto top_moves = std::array<uint16_t, output_size * TestSetSize>{};
  std::generate(begin(top_moves), end(top_moves),
                [i = 0]() mutable { return i++; });
  auto nn_out = std::array<std::array<real_t, output_size>, TestSetSize>{};
  for (auto const& [i, fen] : utl::enumerate(fens)) {
    auto const p = position::from_fen(fen);
    nn_out[i] = nn.estimate(nn_input_from_position(p));
  }
  auto const sort_moves_by_error = [&](auto const it) {
    std::sort(begin(top_moves), it, [&](uint16_t const a, uint16_t const b) {
      auto test_instance_idx_a = a / output_size;
      auto move_idx_a = a % output_size;
      auto test_instance_idx_b = b / output_size;
      auto move_idx_b = b % output_size;
      return std::abs(expected[test_instance_idx_a][move_idx_a] -
                      nn_out[test_instance_idx_a][move_idx_a]) >
             std::abs(expected[test_instance_idx_b][move_idx_b] -
                      nn_out[test_instance_idx_b][move_idx_b]);
    });
  };
  auto const print = [&](auto const it) {
    std::cout << std::left << std::setw(82) << "fen  "
              << "move  " << std::setw(12) << "error  "
              << "stockfish  "
              << "nn"
              << "\n";
    for (auto i = begin(top_moves); i != it; ++i) {
      auto test_instance_idx = *i / output_size;
      auto move_idx = *i % output_size;
      auto from = move_idx % 64;
      auto to = move_idx / 64;
      std::cout << std::left << std::setw(80) << fens[test_instance_idx] << "  "
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

TEST_CASE("train from pgn only startpos") {
  auto const learning_rate = 1.;
  auto const inner_loop_size = 100000;

  auto pl_absolute_errors = plot{""};
  auto pl_max_errors = plot{""};

  auto n = std::make_unique<network<input_size, 32, output_size>>(illegal, 1.0);
  auto p = position::from_fen(start_position_fen);

  constexpr auto const batch_size = 4;
  auto const moves =
      std::array<std::string_view, batch_size - 1>{"e2e4", "e7e5", "g1f3"};
  auto input = std::array<std::array<real_t, input_size>, batch_size>{};
  auto expected = std::array<std::array<real_t, output_size>, batch_size>{};

  input[0] = nn_input_from_position(p);
  expected[0] = to_expected(stockfish_evals(p));

  for (auto const& [i, m] : utl::enumerate(moves)) {
    p.make_move(move{p, std::string{m}}, nullptr);
    input[i + 1] = nn_input_from_position(p);
    expected[i + 1] = to_expected(stockfish_evals(p));
  }

  auto const determine_error = [&]() {
    auto error = std::pair<real_t, real_t>{};
    auto& [e1, e2] = error;
    for (auto batch_idx = 0U; batch_idx != batch_size; ++batch_idx) {
      auto const output = n->estimate(input[batch_idx]);
      for (auto output_idx = 0U; output_idx != output.size(); ++output_idx) {
        auto const diff = output[output_idx] - expected[batch_idx][output_idx];
        e1 += diff * diff;
        e2 = std::max(e2, diff);
      }
    }
    return error;
  };

  for (auto i = 0; i < inner_loop_size; i++) {
    n->train_epoch(input, expected, learning_rate);
    if (i != 0 && i % (inner_loop_size / 100) == 0) {
      const auto [e1, e2] = determine_error();
      pl_absolute_errors.add_entry(i, e1);
      pl_max_errors.add_entry(i, e2);
    }
  }

  auto const print_best_moves = [&]() {
    auto nn_evals = n->estimate(nn_input_from_position(p));
    auto best_moves = std::array<uint16_t, output_size>{};
    std::generate(begin(best_moves), end(best_moves),
                  [i = 0]() mutable { return i++; });
    std::sort(begin(best_moves), end(best_moves),
              [&](uint16_t const a, uint16_t const b) {
                return nn_evals[a] > nn_evals[b];
              });
    for (auto i = begin(best_moves); i != begin(best_moves) + 25; ++i) {
      auto from = *i % 64;
      auto to = *i / 64;
      auto e = nn_evals[*i];
      std::cout << (get_square_name(bitboard{1} << from))
                << (get_square_name(bitboard{1} << to)) << ": " << e << "\n";
    }
  };

  auto test_fens = std::array<std::string, batch_size>{};
  p = position::from_fen(start_position_fen);
  test_fens[0] = p.to_fen();
  for (auto const& [i, m] : utl::enumerate(moves)) {
    p.make_move(move{p, std::string{m}}, nullptr);
    test_fens[i + 1] = p.to_fen();
  }

  print_move_evals_sorted_by_error(printed_errors, *n, test_fens,
                                   stockfish_evals_test_set(test_fens));
  pl_absolute_errors.do_plot();
  pl_max_errors.do_plot();
}