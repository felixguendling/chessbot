#pragma once

#include <memory>
#include <vector>

#include "chessbot/position.h"

namespace chessbot {

struct test_position : public position {
  explicit test_position(std::string const& fen)
      : position{position::from_fen(start_position_fen)} {}

  void make_move(std::string const& s) {
    auto const& m =
        states_.emplace_back(std::make_unique<state_info>(position::make_move(
            move{s}, states_.empty() ? nullptr : states_.back().get())));
  }

  unsigned count_repetitions() const {
    return chessbot::count_repetitions(*this, states_.back().get());
  }

  void print_trace() const { position::print_trace(states_.back().get()); }

  std::vector<std::unique_ptr<state_info>> states_;
};

}  // namespace chessbot