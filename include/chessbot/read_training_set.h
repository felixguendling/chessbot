#pragma once

#include <fstream>
#include <istream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "chessbot/position.h"
#include "chessbot/stockfish_evals.h"

namespace chessbot {

std::vector<std::pair<position, std::map<std::string, move_eval>>>
read_training_set(std::istream& in) {
  auto line = std::string{};
  auto positions =
      std::vector<std::pair<position, std::map<std::string, move_eval>>>{};
  while (std::getline(in, line)) {
    auto p = position{};
    auto l = std::stringstream{line};
    l >> p;
    auto move = std::string{};
    auto eval = std::string{};
    auto moves = std::map<std::string, move_eval>{};
    while (l >> move >> eval) {
      auto m = move_eval{};
      if (!eval.empty() && eval[0] == 'M') {
        eval = eval.substr(1);
        m.mate_ = std::stoi(eval);
      } else {
        m.cp_ = std::stoi(eval);
      }
      moves.emplace(move, m);
    }
    positions.emplace_back(p, moves);
  }
  return positions;
}

}  // namespace chessbot
