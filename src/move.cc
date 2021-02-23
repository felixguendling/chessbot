#include "chessbot/move.h"

#include <ostream>

#include "chessbot/constants.h"
#include "chessbot/util.h"

namespace chessbot {

std::string move::to_str() const {
  if (special_move_ == special_move::CASTLE) {
    auto const is_short_castle = full_file_bitboard(FH) & to();
    auto const from_bb = from();
    auto const to_bb = to();
    auto fixed_to_bb = bitboard{};
    switch (to_bb) {
      case rank_file_to_bitboard(R1, FH):
        fixed_to_bb = rank_file_to_bitboard(R1, FG);
        break;
      case rank_file_to_bitboard(R8, FH):
        fixed_to_bb = rank_file_to_bitboard(R8, FG);
        break;
      case rank_file_to_bitboard(R1, FA):
        fixed_to_bb = rank_file_to_bitboard(R1, FC);
        break;
      case rank_file_to_bitboard(R8, FA):
        fixed_to_bb = rank_file_to_bitboard(R8, FG);
        break;
      default: break;
    }
    return move{from_bb, fixed_to_bb}.to_str();
  }

  auto s = get_square_name(from()) + get_square_name(to());
  if (special_move_ == special_move::PROMOTION) {
    switch (promotion_piece_type_) {
      case promotion_piece_type::QUEEN: s += "=Q"; break;
      case promotion_piece_type::BISHOP: s += "=B"; break;
      case promotion_piece_type::KNIGHT: s += "=N"; break;
      case promotion_piece_type::ROOK: s += "=R"; break;
    }
  }
  return s;
}

std::ostream& operator<<(std::ostream& out, move const m) {
  return out << m.to_str();
}

}  // namespace chessbot