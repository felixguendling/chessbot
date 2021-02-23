#include <atomic>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "chessbot/generate_moves.h"
#include "chessbot/position.h"
#include "chessbot/timing.h"

using namespace chessbot;

uint64_t dfs(position& p, unsigned const current_depth,
             unsigned const max_depth, state_info const* const info_ptr) {
  auto const current_pos_hash = p.get_hash();
  if (current_depth == max_depth || p.half_move_clock_ == 100 ||
      count_repetitions(p, info_ptr, current_pos_hash) >= 3U) {
    return 1U;
  }

  auto leaf_nodes = uint64_t{0U};
  auto const moves_possible = generate_moves(p, [&](move const m) {
    auto const s = p.make_move(m, info_ptr, current_pos_hash);
    auto const leaves = dfs(p, current_depth + 1, max_depth, &s);

    if (current_depth == 0U) {
      std::cout << m << " " << leaves << "\n";
    }

    leaf_nodes += leaves;
    p.undo_move(s);
  });

  if (!moves_possible && !is_valid_move(p, move{0, 0})) {
    // check mate
    //    return 1U;
  }

  return leaf_nodes;
}

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cout << "usage: " << argv[0] << " DEPTH FEN [MOVE, ...]\n";
    return 1;
  }

  auto const depth = std::stoi(argv[1]);
  auto const fen = argv[2];

  auto in = std::stringstream{fen};
  auto p = chessbot::position{};
  in >> p;

  std::vector<std::unique_ptr<state_info>> initial_move_states;
  auto const prev_state_info = [&]() -> state_info const* {
    return initial_move_states.empty() ? nullptr
                                       : initial_move_states.back().get();
  };

  for (auto i = 3; i < argc; ++i) {
    initial_move_states.emplace_back(
        std::make_unique<state_info>(p.make_move(argv[i], prev_state_info())));
  }

  CHESSBOT_START_TIMING(dfs);
  auto const result = dfs(p, 0, depth, prev_state_info());
  CHESSBOT_STOP_TIMING(dfs);
  std::cout << "\n" << result << "\n";
  //  std::cout << CHESSBOT_TIMING_MS(dfs) << "ms\n";
}
