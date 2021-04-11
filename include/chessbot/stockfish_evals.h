#pragma once

#include <map>
#include <string>

#include "chessbot/position.h"

namespace chessbot {

std::map<std::string, float> stockfish_evals(position const&);

}  // namespace chessbot