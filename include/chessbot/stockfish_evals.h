#pragma once

#include <map>
#include <string>

#include "chessbot/position.h"
#include "chessbot/real_t.h"
#include "chessbot/sigmoid.h"

namespace chessbot {

struct move_eval {
  friend bool operator<(move_eval const& a, move_eval const& b) {
    if (a.mate_ == 0 && b.mate_ == 0) {
      return a.cp_ < b.cp_;
    } else if (a.mate_ == 0 && b.mate_ != 0) {
      return b.mate_ > 0;
    } else if (a.mate_ != 0 && b.mate_ == 0) {
      return a.mate_ < 0;
    } else /* if (a.mate_ != 0 && b.mate_ != 0) */ {
      if (a.mate_ > 0 && b.mate_ > 0) {
        return a.mate_ > b.mate_;
      } else if (a.mate_ < 0 && b.mate_ < 0) {
        return a.mate_ < b.mate_;
      } else /* if (a.mate_ > 0 && b.mate_ < 0 || a.mate_ < 0 && b.mate_ > 0) */
      {
        return a.mate_ < b.mate_;
      }
    }
  }

  real_t to_eval() const {
    if (mate_ != 0) {
      return mate_ < 0 ? -0.1 : real_t{1} + (mate_ / real_t{10});
    } else {
      return sigmoid(cp_ / 200.0);
    }
  }

  int mate_{0}, cp_{0};
};

std::map<std::string, move_eval> stockfish_evals(position const&);

}  // namespace chessbot
