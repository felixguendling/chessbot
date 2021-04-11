#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "chessbot/dfs.h"
#include "chessbot/position.h"
#include "chessbot/timing.h"

using namespace chessbot;

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cout << "usage: " << argv[0] << " DEPTH FEN [MOVE, ...]\n";
    return 1;
  }

  auto const depth = std::stoi(argv[1]);
  auto const fen =
      std::string_view{argv[2]} == "startpos" ? start_position_fen : argv[2];

  auto in = std::stringstream{fen};
  auto p = chessbot::position{};
  in >> p;

  std::vector<std::unique_ptr<state_info>> initial_move_states;
  auto const prev_state_info = [&]() -> state_info const* {
    return initial_move_states.empty() ? nullptr
                                       : initial_move_states.back().get();
  };

  std::stringstream moves;
  for (auto i = 3; i < argc; ++i) {
    moves << argv[3];
  }

  std::string move_str;
  while (moves >> move_str) {
    initial_move_states.emplace_back(std::make_unique<state_info>(
        p.make_move(move{p, move_str}, prev_state_info())));
  }

  CHESSBOT_START_TIMING(dfs_rec);
  auto const result = dfs_rec<true>(p, 0U, depth, prev_state_info());
  CHESSBOT_STOP_TIMING(dfs_rec);
  std::cout << "\n" << result << "\n";
  std::cout << CHESSBOT_TIMING_MS(dfs_rec) << "ms\n";
}
