#pragma once

#include <cinttypes>
#include <limits>

namespace chessbot {
using square_t = uint8_t;
constexpr auto const unknown_square = std::numeric_limits<square_t>::max();
}  // namespace chessbot