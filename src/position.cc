#include "chessbot/position.h"

#include <istream>
#include <ostream>
#include <sstream>

#include "fmt/color.h"

#include "utl/enumerate.h"
#include "utl/verify.h"

#include "chessbot/move.h"
#include "chessbot/util.h"

namespace chessbot {

std::string position::to_fen() const {
  std::stringstream ss;
  auto empty_square_count = 0U;
  for (auto rank = 0U; rank != 8; ++rank) {
    for (auto file = 0U; file != 8; ++file) {
      auto empty = true;
      for (auto const& [i, piece] : utl::enumerate(piece_states_)) {
        if ((rank_file_to_bitboard(rank, file) & piece) != 0U) {
          if (empty_square_count != 0U) {
            ss << empty_square_count;
            empty_square_count = 0U;
          }
          ss << (i < NUM_PIECE_TYPES ? white_pieces[i]
                                     : black_pieces[i % NUM_PIECE_TYPES]);
          empty = false;
        }
      }
      if (empty) {
        ++empty_square_count;
      }
    }

    if (empty_square_count != 0U) {
      ss << empty_square_count;
      empty_square_count = 0U;
    }

    if (rank != 7U) {
      ss << '/';
    }
  }

  ss << ' ' << (to_move_ == color::WHITE ? 'w' : 'b') << ' ';

  if (!white_can_short_castle_ && !white_can_long_castle_ &&
      !black_can_short_castle_ && !black_can_long_castle_) {
    ss << '-';
  } else {
    if (white_can_short_castle_) {
      ss << 'K';
    }
    if (white_can_long_castle_) {
      ss << 'Q';
    }
    if (black_can_short_castle_) {
      ss << 'k';
    }
    if (black_can_long_castle_) {
      ss << 'q';
    }
  }

  ss << ' ' << (en_passant_ == 0U ? "-" : get_square_name(en_passant_)) << ' '
     << half_move_clock_ << ' ' << full_move_count_;

  return ss.str();
}

void position::print() const {
  bool white = true;
  fmt::print(" a|b|c|d|e|f|g|h|\n");
  for (auto rank = 0U; rank != 8; ++rank) {
    fmt::print("{}", 8 - rank);
    for (auto file = 0U; file != 8; ++file) {
      auto empty = true;
      for (auto const& [i, piece] : utl::enumerate(piece_states_)) {
        if ((rank_file_to_bitboard(rank, file) & piece) != 0U) {
          fmt::print(
              fmt::emphasis::bold |
                  (i < NUM_PIECE_TYPES ? fmt::fg(fmt::color::red)
                                       : fmt::fg(fmt::color::blue)) |
                  fmt::bg(white ? fmt::color::white : fmt::color::black),
              "{}",
              utf8_pieces[i < NUM_PIECE_TYPES ? 0 : 1][i % NUM_PIECE_TYPES]);
          empty = false;
        }
      }
      if (empty) {
        fmt::print(fmt::emphasis::bold |
                       fmt::bg(white ? fmt::color::white : fmt::color::black),
                   " ");
      }
      fmt::print("|");
      white = !white;
    }
    white = !white;
    fmt::print("\n");
  }
}

std::ostream& operator<<(std::ostream& out, position const& p) {
  bool white = true;
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
      white = !white;
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
    auto const index = pieces.find(input);
    utl::verify(index != std::string_view::npos, "{} is not a valid piece",
                input);
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

std::string position::to_str() const {
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

position position::make_move(move const& m) const {
  auto next = *this;

  auto const from = m.from();
  auto const to = m.to();

  for (auto& pieces : next.get_pieces(next.to_move_)) {
    if ((pieces & from) != 0U) {
      pieces ^= (from | to);
      break;
    }
  }
  for (auto& pieces : next.get_pieces(next.to_move_ == WHITE ? BLACK : WHITE)) {
    if (pieces & to) {
      pieces ^= to;
      break;
    }
  }
  if (next.en_passant_ & to & next.get_pieces(next.to_move_)[PAWN]) {
    auto const en_passant_capture_field = next.to_move_ == WHITE
                                              ? next.en_passant_ << bitboard{8}
                                              : next.en_passant_ >> bitboard{8};
    next.get_pieces(next.to_move_ == WHITE ? BLACK : WHITE)[PAWN] &=
        ~en_passant_capture_field;
  }
  if ((to & next.get_pieces(next.to_move_)[PAWN]) != 0U &&
      std::abs(static_cast<int>(cista::trailing_zeros(from)) -
               static_cast<int>(cista::trailing_zeros(to))) == 16) {
    next.en_passant_ = next.to_move_ == WHITE ? (from >> 8) : (from << 8);
  } else {
    next.en_passant_ = 0U;
  }
  if (m.special_move_ == move::special_move::PROMOTION) {
    next.get_pieces(next.to_move_)[PAWN] ^= to;
    switch (m.promotion_piece_type_) {
      case move::promotion_piece_type::QUEEN:
        next.get_pieces(next.to_move_)[QUEEN] ^= to;
        break;
      case move::promotion_piece_type::ROOK:
        next.get_pieces(next.to_move_)[ROOK] ^= to;
        break;
      case move::promotion_piece_type::BISHOP:
        next.get_pieces(next.to_move_)[BISHOP] ^= to;
        break;
      case move::promotion_piece_type::KNIGHT:
        next.get_pieces(next.to_move_)[KNIGHT] ^= to;
        break;
    }
  }

  if (next.to_move_ == BLACK) {
    ++next.full_move_count_;
  }

  next.to_move_ = next.to_move_ == WHITE ? BLACK : WHITE;

  return next;
}

}  // namespace chessbot
