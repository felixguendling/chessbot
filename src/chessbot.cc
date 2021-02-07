#include "chessbot/chessbot.h"

#include <algorithm>
#include <istream>
#include <ostream>

#include "utl/enumerate.h"
#include "utl/verify.h"

namespace chessbot {

std::ostream& operator<<(std::ostream& out, position const& p) {
  out << "  | a  | b  | c  | d  | e  | f  | g  | h  |\n";
  for (auto rank = 0U; rank != 8; ++rank) {
    out << (8 - rank) << " |";
    for (auto file = 0U; file != 8; ++file) {
      auto empty = true;
      for (auto const& [i, piece] : utl::enumerate(p.piece_states_)) {
        if ((rank_file_to_bitboard(rank, file) & piece) != 0U) {
          out << ' ' << (i < NUM_PIECE_TYPES ? 'w' : 'b')
              << white_pieces[i % NUM_PIECE_TYPES];
          empty = false;
        }
      }
      if (empty) {
        out << "   ";
      }
      out << " |";
    }
    out << '\n';
  }
  return out;
}

std::istream& operator>>(std::istream& in, position& p) {
  enum read_state {
    PIECE_POSITIONS,
    ACTIVE_COLOR,
    CASTLING_AVAILABILITY,
    EN_PASSANT,
    HALF_MOVE_CLOCK,
    FULLMOVE_NUMBER
  } state{PIECE_POSITIONS};

  auto rank = 0U, file = 0U;
  auto const set_piece_position = [&](char input) {
    auto const is_white = std::isupper(input) != 0;
    auto const pieces = is_white ? white_pieces : black_pieces;
    auto const index_it = std::find(begin(pieces), end(pieces), input);
    utl::verify(index_it != end(pieces), "{} is not a valid piece", input);
    auto const index = std::distance(begin(pieces), index_it);
    auto const mask = rank_file_to_bitboard(rank, file);
    p.piece_states_[(!is_white * NUM_PIECE_TYPES) + index] ^= mask;
  };

  p.white_can_short_castle_ = false;
  p.white_can_long_castle_ = false;
  p.black_can_short_castle_ = false;
  p.black_can_long_castle_ = false;

  auto c = char{};
  auto castling_first_space_read = false;
  auto en_passant_file = 0U;
  enum class en_passant_state {
    NONE,
    FILE_READ
  } en_passant_state{en_passant_state::NONE};
  while (true) {
    switch (state) {
      case PIECE_POSITIONS:
        in >> std::noskipws >> c;
        switch (c) {
          case ' ': state = ACTIVE_COLOR; break;
          case '/':
            ++rank;
            file = 0U;
            break;
          case '1':
          case '2':
          case '3':
          case '4':
          case '5':
          case '6':
          case '7': [[fallthrough]];
          case '8': file += c - '0'; break;

          default:
            set_piece_position(c);
            ++file;
            break;
        }
        break;

      case ACTIVE_COLOR:
        in >> c;
        utl::verify(c == 'w' || c == 'b', "invalid color to move {}", c);
        p.to_move_ = c == 'w' ? color::WHITE : color::BLACK;
        state = CASTLING_AVAILABILITY;
        break;

      case CASTLING_AVAILABILITY:
        in >> std::noskipws >> c;
        switch (c) {
          case ' ':
            if (!castling_first_space_read) {
              castling_first_space_read = true;
            } else {
              state = EN_PASSANT;
            }
            break;
          case '-': state = EN_PASSANT; break;
          case 'K': p.white_can_short_castle_ = true; break;
          case 'Q': p.white_can_long_castle_ = true; break;
          case 'k': p.black_can_short_castle_ = true; break;
          case 'q': p.black_can_long_castle_ = true; break;
          default: utl::verify(false, "invalid castling {}", c); break;
        }
        break;

      case EN_PASSANT:
        in >> std::skipws >> c;
        if (c == '-') {
          p.en_passant_ = 0U;
          state = HALF_MOVE_CLOCK;
        } else if (en_passant_state == en_passant_state::NONE) {
          en_passant_file = c - 'a';
          en_passant_state = en_passant_state::FILE_READ;
        } else {
          p.en_passant_ = rank_file_to_bitboard(8 - (c - '0'), en_passant_file);
          state = HALF_MOVE_CLOCK;
        }
        break;

      case HALF_MOVE_CLOCK:
        in >> std::skipws >> p.half_move_clock_;
        state = FULLMOVE_NUMBER;
        break;

      case FULLMOVE_NUMBER: in >> p.full_move_count_; return in;
    }
  }
}

}  // namespace chessbot
