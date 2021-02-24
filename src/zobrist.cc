#include "chessbot/zobrist.h"

#include <array>

#include "utl/enumerate.h"

#include "chessbot/position.h"
#include "chessbot/util.h"

namespace chessbot {

std::array<std::array<zobrist_t, 2>, 64> zobrist_color_hashes = []() {
  auto ids = std::array<std::array<zobrist_t, 2>, 64>{};
  for (auto& i : ids) {
    i[color::WHITE] = get_random_number();
    i[color::BLACK] = get_random_number();
  }
  return ids;
}();

std::array<std::array<zobrist_t, 6>, 64> zobrist_piece_hashes = []() {
  auto ids = std::array<std::array<zobrist_t, 6>, 64>{};
  for (auto& square : ids) {
    for (auto& pt : square) {
      pt = get_random_number();
    }
  }
  return ids;
}();

std::array<zobrist_t, 4> zobrist_castling_right_hashes = []() {
  auto ids = std::array<zobrist_t, 4>{};
  for (auto& i : ids) {
    i = get_random_number();
  }
  return ids;
}();

std::array<zobrist_t, 8> zobrist_en_passant_hashes = []() {
  auto ids = std::array<zobrist_t, 8>{};
  for (auto& i : ids) {
    i = get_random_number();
  }
  return ids;
}();

zobrist_t compute_hash(position const& p) {
  auto hash = zobrist_t{};

  for (auto const& [pt, piece_state_bb] : utl::enumerate(p.piece_states_)) {
    auto const piece_type_idx = pt;
    for_each_set_bit(piece_state_bb, [&](bitboard const piece_bb) {
      auto const square_idx = cista::trailing_zeros(piece_bb);
      auto const color = p.pieces_by_color_[color::WHITE] & piece_bb
                             ? color::WHITE
                             : color::BLACK;
      hash ^= zobrist_piece_hashes[square_idx][piece_type_idx];
      hash ^= zobrist_color_hashes[square_idx][color];
    });
  }

  if (p.castling_rights_.white_can_short_castle_) {
    hash ^= zobrist_castling_right_hashes[castling_right::WHITE_SHORT];
  }
  if (p.castling_rights_.white_can_long_castle_) {
    hash ^= zobrist_castling_right_hashes[castling_right::WHITE_LONG];
  }
  if (p.castling_rights_.black_can_short_castle_) {
    hash ^= zobrist_castling_right_hashes[castling_right::BLACK_SHORT];
  }
  if (p.castling_rights_.black_can_long_castle_) {
    hash ^= zobrist_castling_right_hashes[castling_right::BLACK_LONG];
  }

  if (p.to_move_ == color::BLACK) {
    hash = ~hash;
  }

  if (p.en_passant_) {
    hash ^= zobrist_en_passant_hashes[p.en_passant_ % 8];
  }

  return hash;
}

}  // namespace chessbot