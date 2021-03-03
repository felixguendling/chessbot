#pragma once

#include "chessbot/generate_moves.h"
#include "chessbot/position.h"

namespace chessbot {

template <bool IsRoot = false>
size_t dfs_rec(position& p, unsigned const current_depth,
               unsigned const max_depth, state_info const* const info_ptr) {
  if (p.half_move_clock_ == 100 || count_repetitions(p, info_ptr) >= 3U) {
    return 1U;
  }

  auto const copy = p;
  auto leaf_nodes = size_t{0U};

  std::array<move, max_moves> move_list;
  auto const begin = &move_list[0];
  auto const end = generate_moves(p, begin);
  if (current_depth + 1 == max_depth) {
    if constexpr (IsRoot) {
      for (auto it = begin; it != end; ++it) {
        printf("%s 1\n", it->to_str().c_str());
      }
    }
    leaf_nodes += (end - begin);
  } else {
    for (auto it = begin; it != end; ++it) {
      auto const s = p.make_move(*it, info_ptr);
      auto const leaves = dfs_rec(p, current_depth + 1, max_depth, &s);
      if constexpr (IsRoot) {
        printf("%s %zu\n", it->to_str().c_str(), leaves);
      }
      leaf_nodes += leaves;
      p = copy;
    }
  }

  return leaf_nodes;
}

}  // namespace chessbot