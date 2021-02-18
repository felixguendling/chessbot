#include "doctest/doctest.h"

#include <iostream>
#include <set>

#include "chessbot/magic.h"
#include "chessbot/move.h"
#include "chessbot/position.h"

using namespace chessbot;

TEST_CASE("bishop attack bitboards") {
  auto const id = cista::trailing_zeros(rank_file_to_bitboard(R4, FE));
  REQUIRE(id == 36);

  auto const matches =
      bishop_attack_bbs[id] ==
      (rank_file_to_bitboard(R5, FD) | rank_file_to_bitboard(R6, FC) |
       rank_file_to_bitboard(R7, FB) | rank_file_to_bitboard(R3, FF) |
       rank_file_to_bitboard(R2, FG) | rank_file_to_bitboard(R5, FF) |
       rank_file_to_bitboard(R6, FG) | rank_file_to_bitboard(R3, FD) |
       rank_file_to_bitboard(R2, FC));

  CHECK(matches);
}

TEST_CASE("rook attack bitboards") {
  auto const id = cista::trailing_zeros(rank_file_to_bitboard(R4, FE));
  REQUIRE(id == 36);

  auto const matches = rook_attack_bbs[id] ==
                       ((full_rank_bitboard(R4) | full_file_bitboard(FE)) &
                        ~edge_bitboard & ~rank_file_to_bitboard(R4, FE));

  CHECK(matches);
}

TEST_CASE("rook attack bitboards") {
  auto const id = cista::trailing_zeros(rank_file_to_bitboard(R8, FA));
  REQUIRE(id == 0);

  auto const matches = rook_attack_bbs[id] ==
                       ((full_rank_bitboard(R8) | full_file_bitboard(FA)) &
                        ~full_rank_bitboard(R1) & ~full_file_bitboard(FH) &
                        ~rank_file_to_bitboard(R8, FA));

  CHECK(matches);
}

TEST_CASE("blocking permutations") {
  auto const set_of_all_permutations = [](bitboard const blocker) {
    std::set<bitboard> permutations;
    for_all_permutations(blocker, [&](auto const permutation) {
      permutations.emplace(permutation);
      return false;
    });
    return permutations;
  };

  CHECK(set_of_all_permutations(1) == std::set<bitboard>{0, 1});
  CHECK(set_of_all_permutations(9) == std::set<bitboard>{0, 1, 8, 9});
  CHECK(set_of_all_permutations(13) ==
        std::set<bitboard>{0, 1, 4, 5, 8, 9, 12, 13});
}

TEST_CASE("rook attack squares a8") {
  auto const matches = magic_rook_attack_squares[0][0] ==
                       ((full_rank_bitboard(R8) | full_file_bitboard(FA)) &
                        (~rank_file_to_bitboard(R8, FA)));
  CHECK(matches);
}

TEST_CASE("rook attack squares e4") {
  auto const matches =
      magic_rook_attack_squares[cista::trailing_zeros(rank_file_to_bitboard(
          R4, FE))][0] == ((full_rank_bitboard(R4) | full_file_bitboard(FE)) &
                           (~rank_file_to_bitboard(R4, FE)));
  CHECK(matches);
}

TEST_CASE("rook attack squares blocked") {
  auto const square_idx = cista::trailing_zeros(rank_file_to_bitboard(R4, FE));
  auto const occupancy =
      rank_file_to_bitboard(R5, FE) | rank_file_to_bitboard(R4, FC) |
      rank_file_to_bitboard(R4, FF) | rank_file_to_bitboard(R3, FE);
  auto const magic_index =
      get_magic_index(occupancy, rook_square_magic_numbers[square_idx]);
  auto const matches = magic_rook_attack_squares[square_idx][magic_index] ==
                       (rank_file_to_bitboard(R4, FD) | occupancy);
  CHECK(matches);
}

TEST_CASE("bishop attack squares blocked") {
  auto const square_idx = cista::trailing_zeros(rank_file_to_bitboard(R4, FE));
  auto const occupancy =
      rank_file_to_bitboard(R5, FF) | rank_file_to_bitboard(R5, FD) |
      rank_file_to_bitboard(R3, FD) | rank_file_to_bitboard(R2, FG);
  auto const magic_index =
      get_magic_index(occupancy, bishop_square_magic_numbers[square_idx]);
  auto const matches = magic_bishop_attack_squares[square_idx][magic_index] ==
                       (rank_file_to_bitboard(R3, FF) | occupancy);
  CHECK(matches);
}