#include <atomic>
#include <sstream>
#include <vector>

#include "doctest/doctest.h"

#include "utl/parallel_for.h"

#include "chessbot/generate_moves.h"
#include "chessbot/position.h"
#include "chessbot/timing.h"

using namespace chessbot;

uint64_t dfs(position& p, unsigned const current_depth,
             unsigned const max_depth, bool const print = false) {
  if (current_depth == max_depth || p.new_repetitions() == 3) {

    if (p.new_repetitions() == 3) {
      std::cout << "REPETITION\n";
    }

    return 1U;
  }

  auto leaf_nodes = uint64_t{0U};
  for_each_possible_move(p, [&](move const m) {
    auto const s = p.make_move(m);
    auto const leaves = dfs(p, current_depth + 1, max_depth);

    if (current_depth == 0U) {
      std::cout << get_square_name(m.from()) << get_square_name(m.to()) << ": "
                << leaves << "\n";
    }

    leaf_nodes += leaves;
    p.undo_move(s);
  });

  return leaf_nodes;
}

TEST_CASE("compute all moves") {
  auto in = std::stringstream{start_position_fen};

  auto p = chessbot::position{};
  in >> p;
  p.make_move("g1f3");
  //  p.make_move("a2a3");
  //  p.make_move("c7c6");
  //  p.make_move("b2b3");
  //  p.make_move("d8b6");
  //  p.make_move("e2e3");
  //  p.make_move("b6e3");

  CHESSBOT_START_TIMING(dfs);
  auto const result = dfs(p, 0, 1);
  CHESSBOT_STOP_TIMING(dfs);
  std::cout << result << "\n";
  std::cout << CHESSBOT_TIMING_MS(dfs) << "ms\n";
}