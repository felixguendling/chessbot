#pragma once

#include <cinttypes>
#include <array>
#include <limits>
#include <string_view>

namespace chessbot {

enum color : bool { WHITE, BLACK };
enum rank { R8, R7, R6, R5, R4, R3, R2, R1 };
enum file { FA, FB, FC, FD, FE, FF, FG, FH };
enum piece_type { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NUM_PIECE_TYPES };
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

inline constexpr bitboard rank_file_to_bitboard(unsigned rank, unsigned file) {
  return bitboard{1U} << bitboard{8U * rank + file};
}

inline constexpr bitboard full_rank_bitboard(unsigned i) {
  return rank_file_to_bitboard(i, R1) | rank_file_to_bitboard(i, R2) |
         rank_file_to_bitboard(i, R3) | rank_file_to_bitboard(i, R4) |
         rank_file_to_bitboard(i, R5) | rank_file_to_bitboard(i, R6) |
         rank_file_to_bitboard(i, R7) | rank_file_to_bitboard(i, R8);
}

inline constexpr bitboard full_file_bitboard(unsigned i) {
  return rank_file_to_bitboard(FA, i) | rank_file_to_bitboard(FB, i) |
         rank_file_to_bitboard(FC, i) | rank_file_to_bitboard(FD, i) |
         rank_file_to_bitboard(FE, i) | rank_file_to_bitboard(FF, i) |
         rank_file_to_bitboard(FG, i) | rank_file_to_bitboard(FH, i);
}

constexpr auto const full_bitboard = std::numeric_limits<bitboard>::max();

constexpr auto const second_rank =
    std::array<bitboard, 2>{full_rank_bitboard(R2), full_rank_bitboard(R7)};

constexpr auto const edge_bitboard =
    full_file_bitboard(FA) | full_file_bitboard(FH) | full_rank_bitboard(R1) |
    full_rank_bitboard(R8);

}  // namespace chessbot