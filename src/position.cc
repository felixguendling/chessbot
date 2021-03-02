#include "chessbot/position.h"

#include <istream>
#include <ostream>
#include <sstream>

#include "fmt/color.h"

#include "utl/enumerate.h"
#include "utl/verify.h"

#include "chessbot/generate_moves.h"
#include "chessbot/util.h"

namespace chessbot {

template <bool InitCheckers>
void init_blockers_and_pinners(position& p, color const c) {
  p.blockers_for_king_[c] = 0U;
  p.pinners_[c] = 0U;
  p.checkers_[c] = 0U;

  auto const opposing_color = c == color::WHITE ? color::BLACK : color::WHITE;
  auto const king_square = p.pieces(c, KING);
  auto const king_square_idx = cista::trailing_zeros(king_square);

  if (InitCheckers) {
    p.checkers_[c] = knight_attacks_by_origin_square[king_square_idx] &
                     p.pieces(opposing_color, KNIGHT);
    p.checkers_[c] |= get_attack_squares<BISHOP>(king_square, p.all_pieces()) &
                      p.pieces(opposing_color, BISHOP);
    p.checkers_[c] |= get_attack_squares<BISHOP>(king_square, p.all_pieces()) &
                      p.pieces(opposing_color, QUEEN);
    p.checkers_[c] |= get_attack_squares<ROOK>(king_square, p.all_pieces()) &
                      p.pieces(opposing_color, ROOK);
    p.checkers_[c] |= get_attack_squares<ROOK>(king_square, p.all_pieces()) &
                      p.pieces(opposing_color, QUEEN);
    p.checkers_[c] |=
        pawn_attacks_bb(p.pieces(c, KING), c) & p.pieces(opposing_color, PAWN);
  }

  for_each_set_bit(
      get_attack_squares<BISHOP>(king_square, bitboard{0}) &
          (p.pieces(opposing_color, BISHOP) | p.pieces(opposing_color, QUEEN)) &
          ~p.checkers_[c],
      [&](bitboard const pinner) {
        p.pinners_[c] |= pinner;
        // TODO for_each_set_bit_index
        p.blockers_for_king_[c] |=
            bishop_line_bb[king_square_idx][cista::trailing_zeros(pinner)] &
            p.all_pieces();
      });
  for_each_set_bit(
      get_attack_squares<ROOK>(king_square, bitboard{0}) &
          (p.pieces(opposing_color, ROOK) | p.pieces(opposing_color, QUEEN)) &
          ~p.checkers_[c],
      [&](bitboard const pinner) {
        p.pinners_[c] |= pinner;
        p.blockers_for_king_[c] |=
            rook_line_bb[king_square_idx][cista::trailing_zeros(pinner)] &
            p.all_pieces();
      });
}

void state_info::print_moves() const {
  for_each_pred(
      [](state_info const& pred) { std::cout << pred.last_move_ << " "; });
}

void position::validate() const {
  auto const all_pieces =
      piece_states_[piece_type::PAWN] | piece_states_[piece_type::KNIGHT] |
      piece_states_[piece_type::BISHOP] | piece_states_[piece_type::ROOK] |
      piece_states_[piece_type::QUEEN] | piece_states_[piece_type::KING];
  auto const all_colors =
      pieces_by_color_[color::WHITE] | pieces_by_color_[color::BLACK];

  for (auto rank = 0U; rank != 8; ++rank) {
    for (auto file = 0U; file != 8; ++file) {
      auto const square_bb = rank_file_to_bitboard(rank, file);
      if ((square_bb & pieces_by_color_[color::WHITE] &
           pieces_by_color_[color::BLACK]) != 0U) {
        utl::verify(false,
                    "{} is occupied by black and white:\n"
                    "-- white:\n{}"
                    "-- black:\n{}",
                    get_square_name(square_bb),
                    bitboard_to_str(pieces_by_color_[color::WHITE]),
                    bitboard_to_str(pieces_by_color_[color::BLACK]));
      }

      auto const pieces_bb = square_bb & all_pieces;
      auto const color_bb = square_bb & all_colors;
      if (pieces_bb != color_bb) {
        utl::verify(
            false,
            "{} is set in color bb but not in pieces bb or vice versa:\n"
            "-- pieces:\n{}"
            "-- color:\n{}",
            get_square_name(square_bb), bitboard_to_str(all_pieces),
            bitboard_to_str(all_colors));
      }

      auto const number_of_pieces_on_square =
          ((piece_states_[piece_type::PAWN] & square_bb) != 0U) +
          ((piece_states_[piece_type::KNIGHT] & square_bb) != 0U) +
          ((piece_states_[piece_type::BISHOP] & square_bb) != 0U) +
          ((piece_states_[piece_type::ROOK] & square_bb) != 0U) +
          ((piece_states_[piece_type::QUEEN] & square_bb) != 0U);
      if (number_of_pieces_on_square > 1) {
        utl::verify(false, "{} pieces on square {}", number_of_pieces_on_square,
                    get_square_name(square_bb));
      }
    }
  }
}

position position::from_fen(std::string const& fen) {
  auto ss = std::stringstream{fen};
  auto p = position{};
  ss >> p;
  return p;
}

std::string position::to_fen() const {
  validate();

  std::stringstream ss;
  auto empty_square_count = 0U;
  for (auto rank = 0U; rank != 8; ++rank) {
    for (auto file = 0U; file != 8; ++file) {
      auto empty = true;
      auto const square_bb = rank_file_to_bitboard(rank, file);
      for (auto const& [i, piece] : utl::enumerate(piece_states_)) {
        if ((square_bb & piece) != 0U) {
          if (empty_square_count != 0U) {
            ss << empty_square_count;
            empty_square_count = 0U;
          }
          ss << ((square_bb & pieces_by_color_[color::WHITE])
                     ? white_pieces[i]
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

  if (!castling_rights_.white_can_short_castle_ &&
      !castling_rights_.white_can_long_castle_ &&
      !castling_rights_.black_can_short_castle_ &&
      !castling_rights_.black_can_long_castle_) {
    ss << '-';
  } else {
    if (castling_rights_.white_can_short_castle_) {
      ss << 'K';
    }
    if (castling_rights_.white_can_long_castle_) {
      ss << 'Q';
    }
    if (castling_rights_.black_can_short_castle_) {
      ss << 'k';
    }
    if (castling_rights_.black_can_long_castle_) {
      ss << 'q';
    }
  }

  ss << ' ' << (en_passant_ == 0U ? "-" : get_square_name(en_passant_)) << ' '
     << static_cast<unsigned>(half_move_clock_) << ' ' << full_move_count_;

  return ss.str();
}

void position::print() const {
  try {
    validate();
  } catch (...) {
    std::cout << "INVALID\n";
  }

  bool white = true;
  fmt::print(" a|b|c|d|e|f|g|h|\n");
  for (auto rank = 0U; rank != 8; ++rank) {
    fmt::print("{}", 8 - rank);
    for (auto file = 0U; file != 8; ++file) {
      auto empty = true;
      auto const square_bb = rank_file_to_bitboard(rank, file);
      for (auto const& [i, piece] : utl::enumerate(piece_states_)) {
        auto const is_white = square_bb & pieces_by_color_[color::WHITE];
        if ((square_bb & piece) != 0U) {
          fmt::print(
              fmt::emphasis::bold |
                  (is_white ? fmt::fg(fmt::color::red)
                            : fmt::fg(fmt::color::blue)) |
                  fmt::bg(white ? fmt::color::white : fmt::color::light_gray),
              "{}", utf8_pieces[is_white ? 0 : 1][i]);
          empty = false;
        }
      }
      if (empty) {
        fmt::print(
            fmt::emphasis::bold |
                fmt::bg(white ? fmt::color::white : fmt::color::light_gray),
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
  p.validate();

  bool white = true;
  out << "  | a  | b  | c  | d  | e  | f  | g  | h  |\n";
  for (auto rank = 0U; rank != 8; ++rank) {
    out << (8 - rank) << " |";
    for (auto file = 0U; file != 8; ++file) {
      auto empty = true;
      auto const square_bb = rank_file_to_bitboard(rank, file);
      for (auto const& [i, piece] : utl::enumerate(p.piece_states_)) {
        auto const is_white = square_bb & p.pieces_by_color_[color::WHITE];
        if ((rank_file_to_bitboard(rank, file) & piece) != 0U) {
          out << ' ' << (is_white ? 'w' : 'b') << white_pieces[i];
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
    p.toggle_pieces(static_cast<piece_type>(index),
                    is_white ? color::WHITE : color::BLACK, mask);
  };

  p.castling_rights_.white_can_short_castle_ = false;
  p.castling_rights_.white_can_long_castle_ = false;
  p.castling_rights_.black_can_short_castle_ = false;
  p.castling_rights_.black_can_long_castle_ = false;

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
          case 'K': p.castling_rights_.white_can_short_castle_ = true; break;
          case 'Q': p.castling_rights_.white_can_long_castle_ = true; break;
          case 'k': p.castling_rights_.black_can_short_castle_ = true; break;
          case 'q': p.castling_rights_.black_can_long_castle_ = true; break;
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

      case HALF_MOVE_CLOCK: {
        auto i = 0U;
        in >> std::skipws >> i;
        p.half_move_clock_ = i;
        state = FULLMOVE_NUMBER;
        break;
      }

      case FULLMOVE_NUMBER:
        in >> p.full_move_count_;
        p.hash_ = compute_hash(p);
        if (std::popcount(p.piece_states_[KING]) == 2) {
          init_blockers_and_pinners<true>(p, color::WHITE);
          init_blockers_and_pinners<true>(p, color::BLACK);
        }
        p.validate();
        return in;
    }
  }
}

std::string position::to_str() const {
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

state_info position::make_move(move const m,
                               state_info const* const prev_state) {
#ifndef NDEBUG
  validate();
#endif

  auto info = state_info{en_passant_,      m,     castling_rights_,
                         half_move_clock_, hash_, prev_state};
  ++half_move_clock_;

  if (en_passant_) {
    hash_ ^= zobrist_en_passant_hashes[en_passant_ % 8];
  }
  en_passant_ = bitboard{};

  auto const from = m.from();
  auto const to = m.to();

  if (from & piece_states_[PAWN]) {
    half_move_clock_ = 0;
  }

  if (m.special_move_ == special_move::CASTLE) {
    half_move_clock_ = 0;
    auto const active_player_first_rank = to_move_ == color::WHITE ? R1 : R8;
    toggle_pieces(KING, to_move_, from);
    toggle_pieces(ROOK, to_move_, to);
    if (to == rank_file_to_bitboard(active_player_first_rank, FA)) {
      toggle_pieces(KING, to_move_,
                    rank_file_to_bitboard(active_player_first_rank, FC));
      toggle_pieces(ROOK, to_move_,
                    rank_file_to_bitboard(active_player_first_rank, FD));
    } else {
      toggle_pieces(KING, to_move_,
                    rank_file_to_bitboard(active_player_first_rank, FG));
      toggle_pieces(ROOK, to_move_,
                    rank_file_to_bitboard(active_player_first_rank, FF));
    }

    if (to_move_ == color::WHITE) {
      castling_rights_.white_can_short_castle_ = false;
      castling_rights_.white_can_long_castle_ = false;
      hash_ ^= zobrist_castling_right_hashes[castling_right::WHITE_SHORT];
      hash_ ^= zobrist_castling_right_hashes[castling_right::WHITE_LONG];
    } else {
      castling_rights_.black_can_short_castle_ = false;
      castling_rights_.black_can_long_castle_ = false;
      hash_ ^= zobrist_castling_right_hashes[castling_right::BLACK_SHORT];
      hash_ ^= zobrist_castling_right_hashes[castling_right::BLACK_LONG];
    }
  } else {
    auto pt = 0U;
    for (auto& pieces : piece_states_) {
      if (pieces & to) {
        info.captured_piece_ = static_cast<piece_type>(pt);
        half_move_clock_ = 0;
        pieces ^= to;
        pieces_by_color_[opposing_color()] ^= to;

        switch (to) {
          case rank_file_to_bitboard(R1, FH):
            hash_ ^= zobrist_castling_right_hashes[castling_right::WHITE_SHORT];
            castling_rights_.white_can_short_castle_ = false;
            break;
          case rank_file_to_bitboard(R8, FH):
            hash_ ^= zobrist_castling_right_hashes[castling_right::BLACK_SHORT];
            castling_rights_.black_can_short_castle_ = false;
            break;
          case rank_file_to_bitboard(R1, FA):
            hash_ ^= zobrist_castling_right_hashes[castling_right::WHITE_LONG];
            castling_rights_.white_can_long_castle_ = false;
            break;
          case rank_file_to_bitboard(R8, FA):
            hash_ ^= zobrist_castling_right_hashes[castling_right::BLACK_LONG];
            castling_rights_.black_can_long_castle_ = false;
            break;
          default: break;
        }

        break;
      }
      ++pt;
    }

    pt = 0U;
    for (auto& pieces : piece_states_) {
      if ((pieces & pieces_by_color_[to_move_] & from) != 0U) {
        toggle_pieces(static_cast<piece_type>(pt), to_move_, from);
        toggle_pieces(static_cast<piece_type>(pt), to_move_, to);
        break;
      }
      ++pt;
    }

    if (info.en_passant_ & to & piece_states_[PAWN]) {
      auto const en_passant_capture_field =
          to_move_ == WHITE ? info.en_passant_ << bitboard{8}
                            : info.en_passant_ >> bitboard{8};
      toggle_pieces(PAWN, to_move_ == WHITE ? BLACK : WHITE,
                    en_passant_capture_field);
    }

    if ((to & pieces(to_move_, PAWN)) &&
        std::abs(static_cast<int>(cista::trailing_zeros(from)) -
                 static_cast<int>(cista::trailing_zeros(to))) == 16) {
      en_passant_ = to_move_ == WHITE ? (from >> 8) : (from << 8);
      hash_ ^=
          zobrist_en_passant_hashes[cista::trailing_zeros(en_passant_) % 8];
    }

    if (m.special_move_ == special_move::PROMOTION) {
      toggle_pieces(PAWN, to_move_, to);
      switch (m.promotion_piece_type_) {
        case promotion_piece_type::QUEEN:
          toggle_pieces(QUEEN, to_move_, to);
          break;
        case promotion_piece_type::ROOK:
          toggle_pieces(ROOK, to_move_, to);
          break;
        case promotion_piece_type::BISHOP:
          toggle_pieces(BISHOP, to_move_, to);
          break;
        case promotion_piece_type::KNIGHT:
          toggle_pieces(KNIGHT, to_move_, to);
          break;
      }
    }

    if (to & pieces(to_move_, ROOK)) {
      if (from & rank_file_to_bitboard(R1, FH)) {
        if (castling_rights_.white_can_short_castle_) {
          castling_rights_.white_can_short_castle_ = false;
          hash_ ^= zobrist_castling_right_hashes[castling_right::WHITE_SHORT];
          half_move_clock_ = 0;
        }
      }
      if (from & rank_file_to_bitboard(R1, FA)) {
        if (castling_rights_.white_can_long_castle_) {
          castling_rights_.white_can_long_castle_ = false;
          hash_ ^= zobrist_castling_right_hashes[castling_right::WHITE_LONG];
          half_move_clock_ = 0;
        }
      }
      if (from & rank_file_to_bitboard(R8, FH)) {
        if (castling_rights_.black_can_short_castle_) {
          castling_rights_.black_can_short_castle_ = false;
          hash_ ^= zobrist_castling_right_hashes[castling_right::BLACK_SHORT];
          half_move_clock_ = 0;
        }
      }
      if (from & rank_file_to_bitboard(R8, FA)) {
        if (castling_rights_.black_can_long_castle_) {
          castling_rights_.black_can_long_castle_ = false;
          hash_ ^= zobrist_castling_right_hashes[castling_right::BLACK_LONG];
          half_move_clock_ = 0;
        }
      }
    }

    if (to & pieces(to_move_, KING)) {
      if (to_move_ == color::WHITE) {
        if (castling_rights_.white_can_short_castle_) {
          castling_rights_.white_can_short_castle_ = false;
          hash_ ^= zobrist_castling_right_hashes[castling_right::WHITE_SHORT];
          half_move_clock_ = 0;
        }

        if (castling_rights_.white_can_long_castle_) {
          castling_rights_.white_can_long_castle_ = false;
          hash_ ^= zobrist_castling_right_hashes[castling_right::WHITE_LONG];
          half_move_clock_ = 0;
        }
      } else {
        if (castling_rights_.black_can_short_castle_) {
          castling_rights_.black_can_short_castle_ = false;
          hash_ ^= zobrist_castling_right_hashes[castling_right::BLACK_SHORT];
          half_move_clock_ = 0;
        }
        if (castling_rights_.black_can_long_castle_) {
          castling_rights_.black_can_long_castle_ = false;
          hash_ ^= zobrist_castling_right_hashes[castling_right::BLACK_LONG];
          half_move_clock_ = 0;
        }
      }
    }
  }

  update_blockers_and_pinners(m, info.en_passant_);

  if (to_move_ == BLACK) {
    ++full_move_count_;
  }

  to_move_ = opposing_color();
  hash_ = ~hash_;

  return info;
}

void position::update_blockers_and_pinners(move const m,
                                           bitboard const en_passant) {
  auto const from = m.from();
  auto const to = m.to();

  auto const opposing_king_square_idx =
      cista::trailing_zeros(pieces(opposing_color(), KING));
  const auto our_queens = pieces(to_move_, QUEEN);
  const auto our_bishops = pieces(to_move_, BISHOP);
  auto const our_rooks = pieces(to_move_, ROOK);

  // Pinner was moved or captured.
  // -> remove new non-blockers from blockers list for opposing king
  auto const remove_pinner = [&](color const c,
                                 bitboard const removed_pinner_bb,
                                 unsigned const removed_pinner_square_idx) {
    if (pinners_[c] & removed_pinner_bb) {
      pinners_[c] ^= removed_pinner_bb;

      auto const line_to_edge_bb =
          (bishop_line_to_edge_bb[opposing_king_square_idx]
                                 [removed_pinner_square_idx] |
           rook_line_to_edge_bb[opposing_king_square_idx]
                               [removed_pinner_square_idx]);

      if (!(blockers_for_king_[c] & removed_pinner_bb)) {
        // Piece was a pinner but not a blocker (last in line).
        blockers_for_king_[c] &= ~line_to_edge_bb;

        for_each_set_bit(
            line_to_edge_bb & pinners_[c],
            [&](bitboard const remaining_pinner) {
              blockers_for_king_[c] |=
                  (bishop_line_bb[opposing_king_square_idx]
                                 [cista::trailing_zeros(remaining_pinner)] |
                   rook_line_bb[opposing_king_square_idx]
                               [cista::trailing_zeros(remaining_pinner)]) &
                  all_pieces();
            });
      }
    }
  };

  // Blocker for opposing king was moved.
  // -> remove from blocker bitboard
  // -> check if it's a discovered check for the opposing king
  auto const blocker_for_opposing_king_move = [&]() {
    if (blockers_for_king_[opposing_color()] & from) {
      blockers_for_king_[opposing_color()] ^= from;

      auto const line_to_edge_bb =
          bishop_line_to_edge_bb[opposing_king_square_idx][m.from_field_] |
          rook_line_to_edge_bb[opposing_king_square_idx][m.from_field_];
      if (line_to_edge_bb & to) {
        blockers_for_king_[opposing_color()] |= to;
        return;
      }

      auto const pinners = line_to_edge_bb & pinners_[opposing_color()];
      for_each_set_bit(pinners, [&](bitboard const pinner) {
        auto const pinner_square_idx = cista::trailing_zeros(pinner);
        auto const line_bb =
            bishop_line_bb[opposing_king_square_idx][pinner_square_idx] |
            rook_line_bb[opposing_king_square_idx][pinner_square_idx];
        if (!(line_bb & blockers_for_king_[opposing_color()])) {
          // nothing between pinner and opposing king
          // -> it's a check
          checkers_[opposing_color()] |= pinner;
          pinners_[opposing_color()] ^= pinner;
        }
      });
    }
  };

  // Check if moved piece becomes a blocker for the opposing king.
  auto const becomes_blocker_for_opposing_king = [&]() {
    auto const line_to_opposing_king =
        bishop_line_to_edge_bb[opposing_king_square_idx][m.to_field_] |
        rook_line_to_edge_bb[opposing_king_square_idx][m.to_field_];
    if (line_to_opposing_king &&
        (line_to_opposing_king &
         ~(bishop_line_bb[opposing_king_square_idx][m.to_field_] |
           rook_line_bb[opposing_king_square_idx][m.to_field_]) &
         pinners_[opposing_color()])) {
      blockers_for_king_[opposing_color()] |= to;
    }
  };

  if ((m.to() & piece_states_[KING]) ||
      m.special_move_ == special_move::CASTLE) {
    init_blockers_and_pinners<false>(*this, to_move_);
    blocker_for_opposing_king_move();
    becomes_blocker_for_opposing_king();
    checkers_[to_move_] = 0;
    return;
  }

  // Move own pinner.
  remove_pinner(opposing_color(), from, m.from_field_);

  // Capture opposing pinner.
  remove_pinner(to_move_, to, m.to_field_);

  blocker_for_opposing_king_move();

  // Blocker for own king was moved.
  // -> remove blocker from bitboard
  // -> cannot be a discovered check for own king (rules)
  if (blockers_for_king_[to_move_] & from) {
    blockers_for_king_[to_move_] ^= from;
  }

  becomes_blocker_for_opposing_king();

  // Check if moved piece becomes a blocker for our king.
  // (-> previous attacker becomes pinner in this case)
  auto const own_king_square_idx =
      cista::trailing_zeros(pieces(to_move_, KING));
  auto const line_to_our_king =
      bishop_line_to_edge_bb[own_king_square_idx][m.to_field_] |
      rook_line_to_edge_bb[own_king_square_idx][m.to_field_];
  if (line_to_our_king) {
    // Sliders on the same line to our king as the destination of the move
    // excluding pieces between king and destination.
    auto const attackers =
        ~to & (line_to_our_king &
               ~(bishop_line_bb[opposing_king_square_idx][m.to_field_] |
                 rook_line_bb[own_king_square_idx][m.to_field_]) &
               (pinners_[to_move_] | checkers_[to_move_]));

    // If there is a attacker on the same line
    // -> the piece becomes pinned / blocker for our king
    if (attackers) {
      blockers_for_king_[to_move_] |= to;
    }

    // Previous checker becomes pinner.
    if (checkers_[to_move_] & attackers) {
      pinners_[to_move_] |= attackers;
    }
  }

  // Check if moved piece becomes a blocker for opposing king.
  // (-> previous attacker becomes pinner in this case)
  auto const line_to_oppsing_king =
      bishop_line_to_edge_bb[opposing_king_square_idx][m.to_field_] |
      rook_line_to_edge_bb[opposing_king_square_idx][m.to_field_];
  if (line_to_oppsing_king) {
    // Sliders on the same line to our king as the destination of the move
    // excluding pieces between king and destination.
    auto const attackers =
        line_to_oppsing_king &
        ~(bishop_line_bb[opposing_king_square_idx][m.to_field_] |
          rook_line_to_edge_bb[opposing_king_square_idx][m.to_field_]) &
        pinners_[opposing_color()];

    // If there is a attacker on the same line
    // -> the piece becomes pinned / blocker for our king
    if (attackers) {
      blockers_for_king_[opposing_color()] |= to;
    }
  }

  // Update if diagonal mover is on same diagonal as opposing king.
  if (to & (our_queens | our_bishops)) {
    auto const bishop_attack_line =
        bishop_line_bb[opposing_king_square_idx][m.to_field_];
    if (bishop_attack_line) {
      auto const blockers = bishop_attack_line & all_pieces();
      if (blockers) {
        blockers_for_king_[opposing_color()] |= blockers;
        pinners_[opposing_color()] |= to;
      } else {
        checkers_[opposing_color()] |= to;
      }
    } else if (bishop_line_to_edge_bb[opposing_king_square_idx][m.to_field_]) {
      // bishop next to king not covered by rook_line_bb
      checkers_[opposing_color()] |= to;
    }
  }

  // Update if straight mover is on same line as opposing king.
  if (to & (our_queens | our_rooks)) {
    auto const rook_attack_line =
        rook_line_bb[opposing_king_square_idx][m.to_field_];
    if (rook_attack_line) {
      auto const blockers = rook_attack_line & all_pieces();
      if (blockers) {
        blockers_for_king_[opposing_color()] |= blockers;
        pinners_[opposing_color()] |= to;
      } else {
        checkers_[opposing_color()] |= to;
      }
    } else if (rook_line_to_edge_bb[opposing_king_square_idx][m.to_field_]) {
      // rook next to king not covered by rook_line_bb
      checkers_[opposing_color()] |= to;
    }
  }

  // Update checkers if knight moves.
  if ((to & pieces(to_move_, KNIGHT)) &&
      (knight_attacks_by_origin_square[m.to_field_] &
       pieces(opposing_color(), KING))) {
    checkers_[opposing_color()] |= to;
  }

  // Update checkers if pawn moves.
  if (to & pieces(to_move_, PAWN) &&
      (pieces(opposing_color(), KING) & pawn_attacks_bb(to, to_move_))) {
    checkers_[opposing_color()] |= to;
  }

  if (en_passant & to & piece_states_[PAWN]) {
    auto const captured_pawn =
        to_move_ == color::WHITE ? en_passant << 8 : en_passant >> 8;
    if (captured_pawn & blockers_for_king_[opposing_color()]) {
      blockers_for_king_[opposing_color()] ^= captured_pawn;

      auto const captured_pawn_square_idx =
          cista::trailing_zeros(captured_pawn);
      auto const line_to_opposing_king =
          bishop_line_to_edge_bb[opposing_king_square_idx]
                                [captured_pawn_square_idx] |
          rook_line_to_edge_bb[opposing_king_square_idx]
                              [captured_pawn_square_idx];
      for_each_set_bit(
          line_to_opposing_king & pinners_[opposing_color()],
          [&](bitboard const pinner) {
            auto const pinner_square_idx = cista::trailing_zeros(pinner);
            auto const line_bb =
                bishop_line_bb[opposing_king_square_idx][pinner_square_idx] |
                rook_line_bb[opposing_king_square_idx][pinner_square_idx];
            if (!(line_bb & blockers_for_king_[opposing_color()])) {
              // nothing between pinner and opposing king
              // -> it's a check
              checkers_[opposing_color()] |= pinner;
              blockers_for_king_[opposing_color()] ^= captured_pawn;
            }
          });
    }

    if (captured_pawn & blockers_for_king_[to_move_]) {
      blockers_for_king_[to_move_] ^= captured_pawn;
    }
  }

  checkers_[to_move_] = bitboard{};
}

void position::print_trace(state_info const* const info) const {
  info->print_moves();
  std::cout << "\n";
}

}  // namespace chessbot
