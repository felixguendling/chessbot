#pragma once

#include <cmath>

namespace chessbot {

inline real_t sigmoid(real_t const t) { return 1.0F / (1.0F + std::exp(-t)); }

}  // namespace chessbot
