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
  if (current_depth == max_depth || p.half_move_clock_ == 100 ||
      count_repetitions(p, info_ptr) >= 3U) {
    return 1U;
  }

  auto const copy = p;
  auto leaf_nodes = size_t{0U};
  generate_moves(p, [&](move const m) {
    if (current_depth + 1 == max_depth) {
      ++leaf_nodes;
    } else {
      auto const s = p.make_move(m, info_ptr);
      auto const leaves = dfs_rec(p, current_depth + 1, max_depth, &s);
      if (IsRoot) {
        printf("%s: %zu\n", m.to_str().c_str(), leaves);
      }
      leaf_nodes += leaves;
      p = copy;
    }
  });
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

size_t dfs_iter(position& initial_pos, unsigned const target_depth,
                state_info const* const initial_state_info) {
  constexpr auto const max_depth = 256U;
  constexpr auto const max_move_list_size = 256U;
  using move_list_t = stack<move, max_move_list_size>;

  auto state_infos = stack<state_info, max_depth>{};
  state_infos.push(state_info{});

  auto leaf_count = size_t{0U};
  auto first_layer_leaf_count = size_t{0U};
  auto positions =
      stack<std::tuple<position, state_info const*, move_list_t>, max_depth>{};

  auto& [init_p, init_s, iml] = positions.push();
  auto& init_move_list = iml;
  init_p = initial_pos;
  init_s = initial_state_info;
  generate_moves(init_p, [&](move const m) { init_move_list.push(m); });

  while (!positions.empty()) {
    auto& [p, s, moves] = positions.top();

    if (positions.size() == target_depth || p.half_move_clock_ == 100 ||
        count_repetitions(p, s) >= 3U) {
      positions.pop();
      state_infos.pop();
      ++leaf_count;
      ++first_layer_leaf_count;
      continue;
    }

    if (moves.empty()) {
      if (positions.size() == 2U) {
        printf("%s: %zu\n", state_infos.top().last_move_.to_str().c_str(),
               first_layer_leaf_count);
        first_layer_leaf_count = 0U;
      }
      positions.pop();
      state_infos.pop();
      continue;
    }

    auto const next_move = moves.pop();
    auto& [next_p, next_s, nml] = positions.push();
    auto& next_move_list = nml;
    next_p = p;
    next_s = &state_infos.push(next_p.make_move(next_move, s));
    generate_moves(next_p, [&](move const m) { next_move_list.push(m); });
  }

  return leaf_count;
}

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

  for (auto i = 3; i < argc; ++i) {
    initial_move_states.emplace_back(std::make_unique<state_info>(
        p.make_move(move{argv[i]}, prev_state_info())));
  }

  CHESSBOT_START_TIMING(dfs_rec);
  auto const result = dfs_rec<true>(p, 0U, depth, prev_state_info());
  //  auto const result = dfs_iter(p, depth, prev_state_info());
  CHESSBOT_STOP_TIMING(dfs_rec);
  std::cout << "\n" << result << "\n";
  std::cout << CHESSBOT_TIMING_MS(dfs_rec) << "ms\n";
}
