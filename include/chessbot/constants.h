#pragma once

#include <cinttypes>
#include <string_view>

namespace chessbot {

enum color : bool { WHITE, BLACK };
enum ranks { R8, R7, R6, R5, R4, R3, R2, R1 };
enum files { FA, FB, FC, FD, FE, FF, FG, FH };
enum pieces { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NUM_PIECE_TYPES };
constexpr auto const white_pieces = std::string_view{"PNBRQK"};
constexpr auto const black_pieces = std::string_view{"pnbrqk"};
using bitboard = uint64_t;

}  // namespace chessbot