#include <atomic>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "chessbot/generate_moves.h"
#include "chessbot/position.h"
#include "chessbot/timing.h"

using namespace chessbot;

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
    if (IsRoot) {
      for (auto it = begin; it != end; ++it) {
        printf("%s 1\n", it->to_str().c_str());
      }
    }
    leaf_nodes += (end - begin);
  } else {
    for (auto it = begin; it != end; ++it) {
      auto const s = p.make_move(*it, info_ptr);
      auto const leaves = dfs_rec(p, current_depth + 1, max_depth, &s);
      if (IsRoot) {
        printf("%s %zu\n", it->to_str().c_str(), leaves);
      }
      leaf_nodes += leaves;
      p = copy;
    }
  }

  return leaf_nodes;
}

template <typename T, size_t Size>
struct stack {
  using container_t = std::array<T, Size>;
  template <typename... Args>
  T& push(Args&&... args) {
    elements_[size_] = T{std::forward<Args>(args)...};
    ++size_;
    return elements_[size_ - 1];
  }

  size_t size() const { return size_; }
  T& pop() {
    assert(!empty());
    return elements_[--size_];
  }
  T& top() {
    assert(!empty());
    return elements_[size_ - 1];
  }
  bool empty() const { return size_ == 0U; }
  typename container_t::iterator begin() { return begin(elements_); }
  typename container_t::iterator end() { return begin(elements_) + size_; }

  std::array<T, Size> elements_{};
  size_t size_{0U};
};

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
  //  std::cout << CHESSBOT_TIMING_MS(dfs_rec) << "ms\n";
}
