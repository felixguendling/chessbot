#pragma once

#include <cmath>

#include "chessbot/real_t.h"

namespace chessbot {

struct sigmoid {
  real_t fn(real_t const x) { return 1.0F / (1.0F + std::exp(-x)); }
  real_t fn_d(real_t const fx, real_t const /* x */) { return fx * (1 - fx); }
};

struct relu {
  real_t fn(real_t const x) { return std::max(real_t{0}, t); }
  real_t fn_d(real_t const fx, real_t const /* x */) { return fx <= 0 ? 0 : 1; }
};

struct tanh {
  real_t fn(real_t const x) { return std::tanh(x); }
  real_t fn_d(real_t const fx, real_t const /* x */) { return 1 - (fx * fx); }
};

struct idendity {
  real_t fn(real_t const x) { return x; }
  real_t fn_d(real_t const fx, real_t const /* x */) { return 1 - (fx * fx); }
};

}  // namespace chessbot
