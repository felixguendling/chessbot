#pragma once

#include <cinttypes>
#include <cmath>
#include <array>
#include <limits>
#include <string_view>

#include "chessbot/bitboard.h"
#include "chessbot/north_west.h"

namespace chessbot {

constexpr auto const start_position_fen =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

enum class result { ONGOING, WHITE_WIN, BLACK_WIN, REMIS };

enum color : bool { WHITE, BLACK };

constexpr auto const max_moves = 256;

template <color Color>
constexpr color opposing_color() {
  if constexpr (Color == color::WHITE) {
    return color::BLACK;
  } else {
    return color::WHITE;
  }
}

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

template <typename Fn>
inline constexpr void for_each_knight_target_square(bitboard const origin,
                                                    Fn&& f) {
  for (auto dir :
       {safe_north_west(origin, 2, 1), safe_north_west(origin, 2, -1),
        safe_north_west(origin, -2, 1), safe_north_west(origin, -2, -1),
        safe_north_west(origin, 1, -2), safe_north_west(origin, 1, 2),
        safe_north_west(origin, -1, 2), safe_north_west(origin, -1, -2)}) {
    if (dir) {
      f(dir);
    }
  }
}

constexpr auto const knight_attacks_by_origin_square = []() {
  std::array<bitboard, 64> knight_attacks_bb;
  for (auto i = 0; i < 64; ++i) {
    auto knight_destinations = bitboard{};
    for_each_knight_target_square(bitboard{1} << i,
                                  [&](bitboard const destination) {
                                    knight_destinations |= destination;
                                  });
    knight_attacks_bb[i] = knight_destinations;
  }
  return knight_attacks_bb;
}();

template <typename Fn>
inline constexpr void for_each_king_target_square(bitboard const origin,
                                                  Fn&& f) {
  for (auto dir :
       {safe_north_west(origin, 1, 0), safe_north_west(origin, 1, -1),
        safe_north_west(origin, 1, 1), safe_north_west(origin, 0, -1),
        safe_north_west(origin, 0, 1), safe_north_west(origin, -1, 1),
        safe_north_west(origin, -1, 0), safe_north_west(origin, -1, -1)}) {
    if (dir) {
      f(dir);
    }
  }
}

constexpr auto const king_attacks_by_origin_square = []() {
  std::array<bitboard, 64> king_attacks_bb;
  for (auto i = 0; i < 64; ++i) {
    auto king_destinations = bitboard{};
    for_each_king_target_square(
        bitboard{1} << i,
        [&](bitboard const destination) { king_destinations |= destination; });
    king_attacks_bb[i] = king_destinations;
  }
  return king_attacks_bb;
}();

constexpr bitboard pawn_attacks_bb(bitboard const origin, color const c) {
  return c == color::WHITE ? (((origin & ~full_file_bitboard(FH)) >> 7) |
                              (origin & ~full_file_bitboard(FA)) >> 9)
                           : (((origin & ~full_file_bitboard(FH)) << 9) |
                              ((origin & ~full_file_bitboard(FA)) << 7));
}

static_assert(pawn_attacks_bb(rank_file_to_bitboard(R4, FE), color::WHITE) ==
              (rank_file_to_bitboard(R5, FF) | rank_file_to_bitboard(R5, FD)));
static_assert(pawn_attacks_bb(rank_file_to_bitboard(R4, FA), color::WHITE) ==
              (rank_file_to_bitboard(R5, FB)));
static_assert(pawn_attacks_bb(rank_file_to_bitboard(R4, FH), color::WHITE) ==
              (rank_file_to_bitboard(R5, FG)));
static_assert(pawn_attacks_bb(rank_file_to_bitboard(R4, FA), color::BLACK) ==
              (rank_file_to_bitboard(R3, FB)));
static_assert(pawn_attacks_bb(rank_file_to_bitboard(R4, FH), color::BLACK) ==
              (rank_file_to_bitboard(R3, FG)));

template <typename T>
constexpr T abs(T const i) {
  if (i < 0) {
    return -i;
  }
  return i;
}

constexpr std::array<std::array<bitboard, 64>, 64> get_line_bb(
    bool const diag, bool const to_edge) {
  auto lines = std::array<std::array<bitboard, 64>, 64>{};
  for (auto sq1 = 0; sq1 < 64; ++sq1) {
    for (auto sq2 = 0; sq2 < 64; ++sq2) {
      auto const rank1 = sq1 / 8;
      auto const file1 = sq1 % 8;
      auto const rank2 = sq2 / 8;
      auto const file2 = sq2 % 8;
      auto line = bitboard{0U};
      if (rank1 == rank2 && !diag) {
        for (auto i = 1; i < (to_edge ? 9 : abs(file1 - file2)) && i <= 8;
             ++i) {
          line |= safe_north_west(bitboard{1} << sq1, 0, sq1 < sq2 ? -i : i);
        }
      }
      if (file1 == file2 && !diag) {
        for (auto i = 1; i < (to_edge ? 9 : abs(rank1 - rank2)) && i <= 8;
             ++i) {
          line |= safe_north_west(bitboard{1} << sq1, sq1 < sq2 ? -i : i, 0);
        }
      }
      if (abs(rank1 - rank2) == abs(file1 - file2) && diag) {
        for (auto i = 1; i < (to_edge ? 9 : abs(rank1 - rank2)) && i <= 8;
             ++i) {
          line |= safe_north_west(bitboard{1} << sq1, rank1 < rank2 ? -i : i,
                                  file1 < file2 ? -i : i);
        }
      }
      lines[sq1][sq2] = line;
    }
  }
  return lines;
}

constexpr auto const rook_line_bb = get_line_bb(false, false);
constexpr auto const bishop_line_bb = get_line_bb(true, false);

constexpr auto const rook_line_to_edge_bb = get_line_bb(false, true);
constexpr auto const bishop_line_to_edge_bb = get_line_bb(true, true);

static_assert(rook_line_bb[0][0] == bitboard{0});
static_assert(rook_line_bb[1][14] == bitboard{0});
static_assert(bishop_line_bb[1][14] == bitboard{0});
static_assert(rook_line_bb[1][4] == bitboard{4 + 8});
static_assert(rook_line_bb[0][16] == bitboard{256});
static_assert(bishop_line_bb[2][20] == rank_file_to_bitboard(R7, FD));
static_assert(bishop_line_to_edge_bb[45][54] ==
              (rank_file_to_bitboard(R2, FG) | rank_file_to_bitboard(R1, FH)));
static_assert(rook_line_to_edge_bb[0][3] ==
              (full_rank_bitboard(R8) & ~rank_file_to_bitboard(R8, FA)));

}  // namespace chessbot