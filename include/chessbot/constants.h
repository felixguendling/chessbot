#pragma once

#include <cinttypes>
#include <array>
#include <string_view>

namespace chessbot {

enum color : bool { WHITE, BLACK };
enum ranks { R8, R7, R6, R5, R4, R3, R2, R1 };
enum files { FA, FB, FC, FD, FE, FF, FG, FH };
enum pieces { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NUM_PIECE_TYPES };
constexpr auto const rank_names = std::string_view{"87654321"};
constexpr auto const file_names = std::string_view{"abcdefgh"};
constexpr auto const white_pieces = std::string_view{"PNBRQK"};
constexpr auto const black_pieces = std::string_view{"pnbrqk"};
constexpr auto const utf8_pieces =
    std::array<std::array<std::string_view, 6>, 2>{
        std::array<std::string_view, 6>{"\u2659", "\u2658", "\u2657", "\u2656",
                                        "\u2655", "\u2654"},
        std::array<std::string_view, 6>{"\u265F", "\u265E", "\u265D", "\u265C",
                                        "\u265B", "\u265A"}};
using bitboard = uint64_t;

}  // namespace chessbot