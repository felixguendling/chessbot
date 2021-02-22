#pragma once

#include <chrono>

#define CHESSBOT_START_TIMING(_X) \
  auto _X##_start = std::chrono::steady_clock::now(), _X##_stop = _X##_start
#define CHESSBOT_STOP_TIMING(_X) _X##_stop = std::chrono::steady_clock::now()
#define CHESSBOT_TIMING_MS(_X)                                       \
  (std::chrono::duration_cast<std::chrono::milliseconds>(_X##_stop - \
                                                         _X##_start) \
       .count())
#define CHESSBOT_TIMING_US(_X)                                       \
  (std::chrono::duration_cast<std::chrono::microseconds>(_X##_stop - \
                                                         _X##_start) \
       .count())