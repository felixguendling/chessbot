#pragma once

#include <cinttypes>
#include <array>
#include <limits>
#include <string_view>

namespace chessbot {

enum color : bool { WHITE, BLACK };
enum rank : unsigned { R8, R7, R6, R5, R4, R3, R2, R1 };
enum file : unsigned { FA, FB, FC, FD, FE, FF, FG, FH };
enum piece_type : uint8_t {
  PAWN,
  KNIGHT,
  BISHOP,
  ROOK,
  QUEEN,
  KING,
  NUM_PIECE_TYPES
};
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

struct rank_file {
  constexpr rank_file(unsigned r, unsigned f) : file_{f}, rank_{r} {}
  file file_;
  rank rank_;
};

inline constexpr bitboard rank_file_to_bitboard(unsigned rank, unsigned file) {
  return bitboard{1U} << bitboard{8U * rank + file};
}

template <typename... Args>
inline constexpr bitboard multi_rank_file_to_bitboard(Args... rf) {
  return (rank_file_to_bitboard(rf.rank_, rf.file_) | ...);
}

inline constexpr bitboard full_rank_bitboard(unsigned i) {
  return multi_rank_file_to_bitboard(
      rank_file{i, R1}, rank_file{i, R2}, rank_file{i, R3}, rank_file{i, R4},
      rank_file{i, R5}, rank_file{i, R6}, rank_file{i, R7}, rank_file{i, R8});
}

inline constexpr bitboard full_file_bitboard(unsigned i) {
  return multi_rank_file_to_bitboard(
      rank_file{FA, i}, rank_file{FB, i}, rank_file{FC, i}, rank_file{FD, i},
      rank_file{FE, i}, rank_file{FF, i}, rank_file{FG, i}, rank_file{FH, i});
}

constexpr auto const full_bitboard = std::numeric_limits<bitboard>::max();

constexpr auto const second_rank =
    std::array<bitboard, 2>{full_rank_bitboard(R2), full_rank_bitboard(R7)};

constexpr auto const edge_bitboard =
    full_file_bitboard(FA) | full_file_bitboard(FH) | full_rank_bitboard(R1) |
    full_rank_bitboard(R8);

constexpr auto const short_castle_rook_traversal_squares =
    std::array{rank_file_to_bitboard(R1, FG) | rank_file_to_bitboard(R1, FF),
               rank_file_to_bitboard(R8, FG) | rank_file_to_bitboard(R8, FF)};

constexpr auto const long_castle_rook_traversal_squares =
    std::array{multi_rank_file_to_bitboard(rank_file{R1, FD}, rank_file{R1, FC},
                                           rank_file{R1, FB}),
               multi_rank_file_to_bitboard(rank_file{R8, FD}, rank_file{R8, FC},
                                           rank_file{R8, FB})};

constexpr auto const short_castle_knight_attack_squares = std::array{
    multi_rank_file_to_bitboard(rank_file{R3, FH}, rank_file{R3, FG},
                                rank_file{R3, FF}, rank_file{R3, FE},
                                rank_file{R3, FD}, rank_file{R2, FH},
                                rank_file{R2, FG}, rank_file{R2, FE},
                                rank_file{R2, FD}, rank_file{R2, FC}),
    multi_rank_file_to_bitboard(rank_file{R6, FH}, rank_file{R6, FG},
                                rank_file{R6, FF}, rank_file{R6, FE},
                                rank_file{R6, FD}, rank_file{R7, FH},
                                rank_file{R7, FG}, rank_file{R7, FE},
                                rank_file{R7, FD}, rank_file{R7, FC})};

constexpr auto const long_castle_knight_attack_squares =
    std::array{multi_rank_file_to_bitboard(
                   rank_file{R3, FB}, rank_file{R3, FC}, rank_file{R3, FD},
                   rank_file{R3, FE}, rank_file{R3, FF}, rank_file{R2, FA},
                   rank_file{R2, FB}, rank_file{R2, FC}, rank_file{R2, FE},
                   rank_file{R2, FE}, rank_file{R2, FF}, rank_file{R2, FG}),
               multi_rank_file_to_bitboard(
                   rank_file{R6, FB}, rank_file{R6, FC}, rank_file{R6, FD},
                   rank_file{R6, FE}, rank_file{R6, FF}, rank_file{R7, FA},
                   rank_file{R7, FB}, rank_file{R7, FC}, rank_file{R7, FE},
                   rank_file{R7, FE}, rank_file{R7, FF}, rank_file{R7, FG})};

constexpr auto const short_castle_pawn_king_attack_squares =
    std::array{multi_rank_file_to_bitboard(rank_file(R2, FH), rank_file(R2, FG),
                                           rank_file(R2, FF), rank_file(R2, FE),
                                           rank_file(R2, FD)),
               multi_rank_file_to_bitboard(rank_file(R7, FH), rank_file(R7, FG),
                                           rank_file(R7, FF), rank_file(R7, FE),
                                           rank_file(R7, FD))};

constexpr auto const long_castle_pawn_king_attack_squares =
    std::array{multi_rank_file_to_bitboard(rank_file(R2, FB), rank_file(R2, FC),
                                           rank_file(R2, FF), rank_file(R2, FE),
                                           rank_file(R2, FD)),
               multi_rank_file_to_bitboard(rank_file(R7, FB), rank_file(R7, FC),
                                           rank_file(R7, FF), rank_file(R7, FE),
                                           rank_file(R7, FD))};

constexpr auto const short_castle_king_traversal_squares = std::array{
    std::array{rank_file_to_bitboard(R1, FE), rank_file_to_bitboard(R1, FF),
               rank_file_to_bitboard(R1, FG)},
    std::array{rank_file_to_bitboard(R8, FE), rank_file_to_bitboard(R8, FF),
               rank_file_to_bitboard(R8, FG)}};

constexpr auto const long_castle_king_traversal_squares = std::array{
    std::array{rank_file_to_bitboard(R1, FD), rank_file_to_bitboard(R1, FC),
               rank_file_to_bitboard(R1, FE)},
    std::array{rank_file_to_bitboard(R8, FD), rank_file_to_bitboard(R8, FC),
               rank_file_to_bitboard(R8, FE)}};

}  // namespace chessbot