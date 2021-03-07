#pragma once

#include <numeric>

#include "chessbot/generate_moves.h"
#include "chessbot/position.h"
#include "chessbot/util.h"

namespace chessbot {

float evaluate(position const& p) { return ((double)rand() / (RAND_MAX)); }

void indent(unsigned const depth) {
  for (auto i = 0; i < depth; ++i) {
    std::cout << "  ";
  }
}

template <bool IsRoot = true, bool PrintDebug = false>
unsigned mcts(position const& p, unsigned const number_of_games,
              state_info const* const info_ptr,
              std::array<move, max_moves>& move_list,
              std::array<float, max_moves>& end_evaluations,
              unsigned const depth = 0U) {
  auto evaluations = std::array<float, max_moves>{};
  auto points = std::array<unsigned, max_moves>{};
  auto contingents = std::array<unsigned, max_moves>{};

  auto const begin = &move_list[0];
  auto const end = generate_moves(p, begin);

  if (begin == end) {
    auto const result =
        p.checkers_[p.to_move_] ? (p.to_move_ == color::WHITE ? 2 : 0) : 1;

    if constexpr (PrintDebug) {
      indent(depth);
      std::cout << result << " [end]\n";
    }

    return result * number_of_games;
  }

  if (p.half_move_clock_ == 4 || count_repetitions(p, info_ptr) >= 2U) {
    if constexpr (PrintDebug) {
      indent(depth);
      std::cout << "1 [repetitions/halfmoveclock]"
                << "\n";
    }
    return 1U * number_of_games;
  }

  auto i = 0U;
  auto next_pos = position{};
  for (auto it = begin; it != end; ++it, ++i) {
    next_pos = p;
    next_pos.make_move(*it, info_ptr);
    evaluations[i] = evaluate(next_pos);
  }

  auto const sum_of_evaluation =
      std::accumulate(std::begin(evaluations), std::end(evaluations), .0);

  i = 0U;
  auto games = 0U;
  auto rests = std::array<float, max_moves>{};
  for (auto it = begin; it != end; ++it, ++i) {
    auto const f = number_of_games * evaluations[i] / sum_of_evaluation;
    contingents[i] = f;
    games += contingents[i];
    rests[i] = f - contingents[i];
  }

  auto indices = std::array<unsigned, max_moves>{};
  std::generate(std::begin(indices), std::begin(indices) + (end - begin),
                [j = 0]() mutable { return j++; });
  std::sort(
      std::begin(indices), std::begin(indices) + (end - begin),
      [&](unsigned const a, unsigned const b) { return rests[a] < rests[b]; });
  for (auto rest_games_i = 0U; rest_games_i < number_of_games - games;
       ++rest_games_i) {
    ++contingents[indices[rest_games_i]];
  }

  if constexpr (IsRoot) {
    for (auto& c : contingents) {
      c = std::max(1U, c);
    }
  }

  i = 0U;
  auto called = false;
  for (auto it = begin; it != end; ++it, ++i) {
    if constexpr (PrintDebug) {
      indent(depth);
      std::cout << move_list[i].to_str();
    }

    if (contingents[i] == 0) {
      if constexpr (PrintDebug) {
        std::cout << " -> no contingent  [" << evaluations[i] << '/'
                  << sum_of_evaluation << "]\n";
      }
      continue;
    }

    if constexpr (PrintDebug) {
      std::cout << " -> accepted [" << evaluations[i] << '/'
                << sum_of_evaluation << "]\n";
    }

    next_pos = p;
    auto child_state_info = next_pos.make_move(move_list[i], info_ptr);
    auto next_moves = std::array<move, max_moves>{};
    auto next_end_evaluations = std::array<float, max_moves>{};
    points[i] =
        mcts<false, PrintDebug>(next_pos, contingents[i], &child_state_info,
                                next_moves, next_end_evaluations, depth + 1);
    called = true;
  }

  if (!called) {
    auto const best_move_it =
        std::max_element(std::begin(evaluations), std::end(evaluations));
    auto const best_move_idx =
        std::distance(std::begin(evaluations), best_move_it);

    next_pos = p;
    auto child_state_info =
        next_pos.make_move(move_list[best_move_idx], info_ptr);
    auto next_moves = std::array<move, max_moves>();
    auto next_end_evaluations = std::array<float, max_moves>();

    if constexpr (PrintDebug) {
      indent(depth);
      std::cout << "nothing accepted, choosing "
                << move_list[best_move_idx].to_str() << "\n";
    }

    contingents[best_move_idx] = 1;
    points[best_move_idx] = mcts<false, PrintDebug>(
        next_pos, contingents[best_move_idx], &child_state_info, next_moves,
        next_end_evaluations, depth + 1);
  }

  if constexpr (PrintDebug) {
    indent(depth);
    i = 0U;
    std::cout << "points:\n";
    for (auto it = begin; it != end; ++it, ++i) {
      indent(depth);
      std::cout << move_list[i].to_str() << ": " << points[i] << "\n";
    }
  }

  if (IsRoot) {
    i = 0U;
    for (auto it = begin; it != end; ++it, ++i) {
      end_evaluations[i] =
          (contingents[i] == 0)
              ? 0
              : static_cast<float>(points[i]) / (2 * contingents[i]);
    }
  }

  return std::accumulate(std::begin(points), std::end(points), 0);
}

}  // namespace chessbot