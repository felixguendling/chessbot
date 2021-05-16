#include "doctest/doctest.h"

#include <iostream>

#include "chessbot/generate_moves.h"
#include "chessbot/nn.h"
#include "chessbot/nn_chess.h"
#include "chessbot/pgn.h"
#include "chessbot/plot.h"
#include "chessbot/position.h"
#include "chessbot/stockfish_evals.h"

using namespace chessbot;

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

  auto const to_expected = [&](std::map<std::string, move_eval> const& evals) {
    auto expected = std::array<real_t, output_size>{};
    for (auto const& [move, eval] : evals) {
      auto const from = name_to_square(move.substr(0, 2));
      auto const to = name_to_square(move.substr(2, 2));
      expected[from + 64 * to] = eval.to_eval();
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

template <size_t TestSetSize>
auto stockfish_evals_test_set(
    std::array<std::string, TestSetSize> const& fens) {
  auto expected = std::array<std::array<real_t, output_size>, TestSetSize>{};
  for (auto const& [i, fen] : utl::enumerate(fens)) {
    expected[i] = to_expected(stockfish_evals(position::from_fen(fen)));
  }
  return expected;
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

  for (auto i = 0; i < inner_loop_size; i++) {
    n->train_epoch(input, expected, learning_rate);
    if (i != 0 && i % (inner_loop_size / 100) == 0) {
      const auto [e1, e2] = determine_error(*n, input, expected);
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
