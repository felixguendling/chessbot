#include "chessbot/move.h"

#include <ostream>

#include "utl/verify.h"

#include "chessbot/constants.h"
#include "chessbot/position.h"
#include "chessbot/util.h"

namespace chessbot {

move::move(position const& p, std::string const& str) : move{0U, 0U} {
  if (str == "0-0w" || (str == "e1g1" && (p.piece_states_[KING] &
                                          rank_file_to_bitboard(R1, FE)))) {
    special_move_ = special_move::CASTLE;
    from_field_ = cista::trailing_zeros(rank_file_to_bitboard(R1, FE));
    to_field_ = cista::trailing_zeros(rank_file_to_bitboard(R1, FH));
    return;
  }
  if (str == "0-0b" || (str == "e8g8" && (p.piece_states_[KING] &
                                          rank_file_to_bitboard(R8, FE)))) {
    special_move_ = special_move::CASTLE;
    from_field_ = cista::trailing_zeros(rank_file_to_bitboard(R8, FE));
    to_field_ = cista::trailing_zeros(rank_file_to_bitboard(R8, FH));
    return;
  }
  if (str == "0-0-0w" || (str == "e1c1" && (p.piece_states_[KING] &
                                            rank_file_to_bitboard(R1, FE)))) {
    special_move_ = special_move::CASTLE;
    from_field_ = cista::trailing_zeros(rank_file_to_bitboard(R1, FE));
    to_field_ = cista::trailing_zeros(rank_file_to_bitboard(R1, FA));
    return;
  }
  if (str == "0-0-0b" || (str == "e8c8" && (p.piece_states_[KING] &
                                            rank_file_to_bitboard(R8, FE)))) {
    special_move_ = special_move::CASTLE;
    from_field_ = cista::trailing_zeros(rank_file_to_bitboard(R8, FE));
    to_field_ = cista::trailing_zeros(rank_file_to_bitboard(R8, FA));
    return;
  }

  utl::verify(str.size() == 4 || str.size() == 5,
              "{} is not a valid move string: length not 4 or 5");

  auto const from_file = str[0] - 'a';
  auto const from_rank = 8 - (str[1] - '0');
  auto const to_file = str[2] - 'a';
  auto const to_rank = 8 - (str[3] - '0');

  from_field_ = 8U * from_rank + from_file;
  to_field_ = 8U * to_rank + to_file;

  if (str.size() == 5) {
    switch (str[4]) {
      case 'Q':
        special_move_ = special_move::PROMOTION;
        promotion_piece_type_ = promotion_piece_type::QUEEN;
        break;
      case 'R':
        special_move_ = special_move::PROMOTION;
        promotion_piece_type_ = promotion_piece_type::ROOK;
        break;
      case 'B':
        special_move_ = special_move::PROMOTION;
        promotion_piece_type_ = promotion_piece_type::BISHOP;
        break;
      case 'N':
        special_move_ = special_move::PROMOTION;
        promotion_piece_type_ = promotion_piece_type::KNIGHT;
        break;
      case 'q':
        special_move_ = special_move::PROMOTION;
        promotion_piece_type_ = promotion_piece_type::QUEEN;
        break;
      case 'r':
        special_move_ = special_move::PROMOTION;
        promotion_piece_type_ = promotion_piece_type::ROOK;
        break;
      case 'b':
        special_move_ = special_move::PROMOTION;
        promotion_piece_type_ = promotion_piece_type::BISHOP;
        break;
      case 'n':
        special_move_ = special_move::PROMOTION;
        promotion_piece_type_ = promotion_piece_type::KNIGHT;
        break;
      default: utl::verify(false, "invalid promotion piece type {}", str[4]);
    }
  }
}

std::string move::to_str() const {
  if (special_move_ == special_move::CASTLE) {
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
        fixed_to_bb = rank_file_to_bitboard(R8, FC);
        break;
      default: break;
    }
    return move{from_bb, fixed_to_bb}.to_str();
  }

  auto s = get_square_name(from()) + get_square_name(to());
  if (special_move_ == special_move::PROMOTION) {
    switch (promotion_piece_type_) {
      case promotion_piece_type::QUEEN: s += "q"; break;
      case promotion_piece_type::BISHOP: s += "b"; break;
      case promotion_piece_type::KNIGHT: s += "n"; break;
      case promotion_piece_type::ROOK: s += "r"; break;
    }
  }
  return s;
}

std::ostream& operator<<(std::ostream& out, move const m) {
  return out << m.to_str();
}

}  // namespace chessbot