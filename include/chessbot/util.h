#pragma once

#include <string>

#include "chessbot/constants.h"

namespace chessbot {

std::string get_square_name(bitboard);
bitboard rank_file_to_bitboard(rank, file);

}  // namespace chessbot