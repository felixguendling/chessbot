#include "chessbot/zobrist.h"

#include "chessbot/util.h"

namespace chessbot {

std::array<std::array<std::array<uint64_t, 6>, 2>, 64> zobrist_piece_hashes =
    []() {
      auto ids = std::array<std::array<std::array<uint64_t, 6>, 2>, 64>{};
      for (auto& square : ids) {
        for (auto& color : square) {
          for (auto& pt : color) {
            pt = get_random_number();
          }
        }
      }
      return ids;
    }();

std::array<zobrist_t, 4> zobrist_castling_rights_hashes = []() {
  auto ids = std::array<zobrist_t, 4>{};
  for (auto& i : ids) {
    i = get_random_number();
  }
  return ids;
}();

std::array<zobrist_t, 8> en_passant_hashes = []() {
  auto ids = std::array<zobrist_t, 8>{};
  for (auto& i : ids) {
    i = get_random_number();
  }
  return ids;
}();

}  // namespace chessbot