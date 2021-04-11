#include "chessbot/pgn.h"

#include <charconv>
#include <ostream>
#include <vector>

#include "utl/parser/cstr.h"
#include "utl/raii.h"
#include "utl/verify.h"

#include "chessbot/util.h"

namespace chessbot {

std::string_view shorten(std::string_view s) {
  return s.substr(0, std::min(s.size(), size_t{500U}));
}

std::ostream& operator<<(std::ostream& out, game::move const& m) {
  if (m.castle_ != game::move::castle::NONE) {
    return out << (m.castle_ == game::move::castle::SHORT ? "O-O" : "O-O-O");
  }
  if (m.piece_ != PAWN) {
    out << white_pieces[m.piece_];
  }
  if (m.from_file_ != game::move::unknown) {
    out << m.from_file_;
  }
  if (m.from_rank_ != game::move::unknown) {
    out << m.from_rank_;
  }
  if (m.from_rank_ != game::move::unknown ||
      m.from_file_ != game::move::unknown) {
    out << '-';
  }
  out << get_square_name(bitboard{1} << m.to_);
  if (m.promotion_piece_type_ != game::move::no_promotion) {
    switch (m.promotion_piece_type_) {
      case promotion_piece_type::BISHOP: out << "=B"; break;
      case promotion_piece_type::ROOK: out << "=R"; break;
      case promotion_piece_type::KNIGHT: out << "=N"; break;
      case promotion_piece_type::QUEEN: out << "=Q"; break;
    }
  }
  return out;
}

game::header parse_header(utl::cstr& pgn) {
  auto h = game::header{};

  auto const parse_header_line = [&]() {
    utl::verify(pgn.length() != 0U && pgn[0] == '[',
                "expected pgn header start [, got \"{}\"", shorten(pgn));
    ++pgn;  // skip '['

    auto const space_pos = pgn.view().find(" \"");
    utl::verify(space_pos != std::string_view::npos,
                "header without space: \"{}\"", shorten(pgn));
    auto const key = pgn.view().substr(0, space_pos);

    pgn += space_pos + 2;

    auto const end_pos = pgn.view().find("\"]");
    utl::verify(end_pos != std::string_view::npos, "header without end \"{}\"",
                shorten(pgn));
    auto const value = pgn.view().substr(0, end_pos);

    pgn += end_pos + 2;

    if (key == "WhiteElo") {
      utl::verify(std::from_chars(begin(value), end(value), h.elo_white_).ec ==
                      std::errc{},
                  "could not parse white elo \"{}\"", value);
    } else if (key == "BlackElo") {
      utl::verify(std::from_chars(begin(value), end(value), h.elo_black_).ec ==
                      std::errc{},
                  "could not parse black elo \"{}\"", value);
    } else if (key == "TimeControl") {
      if (value == "-") {
        h.start_time_ = game::header::infinite_time;
        h.time_increment_ = game::header::infinite_time;
      } else {
        auto const r1 =
            std::from_chars(begin(value), end(value), h.start_time_);
        utl::verify(r1.ec == std::errc{}, "could not parse start time \"{}\"",
                    value);

        auto rest = std::string_view{r1.ptr, end(value)};
        utl::verify(rest.size() >= 0 && rest[0] == '+',
                    "unexpected time control \"{}\"", value);
        rest = rest.substr(1);
        utl::verify(
            std::from_chars(begin(rest), end(rest), h.time_increment_).ec ==
                std::errc{},
            "could not parse time increment \"{}\"", value);
      }
    }
  };

  while (true) {
    parse_header_line();
    utl::verify(pgn[0] == '\n', "header without line break \"{}\"",
                shorten(pgn));
    ++pgn;

    if (pgn[0] == '\n') {
      ++pgn;
      break;
    }
  }

  return h;
}

bool is_result(utl::cstr& pgn) {
  pgn = pgn.skip_whitespace_front();
  for (auto const& r : {"1/2-1/2", "1-0", "0-1", "*"}) {
    if (pgn.starts_with(r)) {
      pgn += std::strlen(r);
      return true;
    }
  }
  return false;
}

void skip_stuff(utl::cstr& pgn) {
  while (!pgn.empty() && (pgn[0] == '?' || pgn[0] == '!' || pgn[0] == '#' ||
                          pgn[0] == '+' || pgn[0] == 'x' || pgn[0] == '=')) {
    ++pgn;
  }
}

game::move parse_move(utl::cstr& pgn) {
  UTL_FINALLY([&]() {
    if (!pgn.empty() && pgn[0] == '=') {
      utl::verify(pgn.length() > 1,
                  "promotion string shorter than expected \"{}\"",
                  shorten(pgn));
      pgn += 2;
    }

    skip_stuff(pgn);

    pgn = pgn.skip_whitespace_front();
    if (!pgn.empty() && pgn[0] == '{') {
      auto const closing_bracket_pos = pgn.view().find('}');
      utl::verify(closing_bracket_pos != std::string::npos,
                  "no closing bracket found \"{}\"", shorten(pgn));
      pgn += closing_bracket_pos + 1;
      pgn = pgn.skip_whitespace_front();
    }
  });
  auto move_end_pos = pgn.view().find(' ');
  if (!pgn.empty() && pgn[0] == 'O') {
    if (pgn.starts_with("O-O-O")) {
      auto const ret = game::move{.castle_ = game::move::castle::LONG};
      pgn += 5;
      return ret;
    } else if (pgn.starts_with("O-O")) {
      auto const ret = game::move{.castle_ = game::move::castle::SHORT};
      pgn += 3;
      return ret;
    } else {
      utl::verify(false, "unexpected castling length \"{}\"",
                  pgn.view().substr(0, move_end_pos));
      return game::move{};
    }
  } else {
    auto const to_piece_type = [](char const c) {
      switch (c) {
        case 'N': return piece_type::KNIGHT;
        case 'B': return piece_type::BISHOP;
        case 'K': return piece_type::KING;
        case 'Q': return piece_type::QUEEN;
        case 'R': return piece_type::ROOK;
        default: return piece_type::PAWN;
      }
    };

    auto const is_file = [](char const c) { return c >= 'a' && c <= 'h'; };

    //    auto const check =
    //        pgn[move_end_pos - 1] == '#' || pgn[move_end_pos - 1] == '+';
    //    auto promotion_pt = static_cast<promotion_piece_type>(
    //        to_piece_type(pgn[move_end_pos - (check ? 2 : 1)]) - 1);

    auto pt = to_piece_type(pgn[0]);
    if (!pgn.empty() && std::isupper(pgn[0])) {
      ++pgn;
    }

    skip_stuff(pgn);

    utl::verify(!pgn.empty(), "expected from field / to field");

    auto m = game::move{.piece_ = pt};
    if (std::isdigit(pgn[0])) {
      m.from_rank_ = pgn[0];
      ++pgn;

      skip_stuff(pgn);

      utl::verify(pgn.length() >= 2 && is_file(pgn[0]) && std::isdigit(pgn[1]),
                  "expected to field after rank, got {}", shorten(pgn));
      m.to_ = name_to_square(pgn.view().substr(0, 2));
      pgn += 2;
    } else if (is_file(pgn[0])) {
      auto const file = pgn[0];
      ++pgn;

      utl::verify(!pgn.empty(), "expected move, got end of pgn");
      skip_stuff(pgn);

      utl::verify(!pgn.empty(), "expected move, got end of pgn");
      if (std::isdigit(pgn[0])) {
        auto const rank = pgn[0];
        ++pgn;
        skip_stuff(pgn);
        if (!pgn.empty() && is_file(pgn[0])) {
          utl::verify(pgn.length() >= 2 && std::isdigit(pgn[1]),
                      "expected to field, got {}", shorten(pgn));
          m.from_file_ = file;
          m.from_rank_ = rank;
          m.to_ = name_to_square(pgn.view().substr(0, 2));
          pgn += 2;
        } else {
          auto const str = std::array<char, 2>{file, rank};
          m.to_ = name_to_square(std::string_view{&str[0], 2});
        }
      } else {
        skip_stuff(pgn);
        utl::verify(
            pgn.length() >= 2 && (is_file(pgn[0]) && std::isdigit(pgn[1])),
            "expected to field, got {}", shorten(pgn));
        m.from_file_ = file;
        m.to_ = name_to_square(pgn.view().substr(0, 2));
        pgn += 2;
      }
    } else {
      utl::verify(false, "expected from field / to field, got {}",
                  shorten(pgn));
    }

    skip_stuff(pgn);

    utl::verify(!pgn.empty(), "unexpected pgn string end");
    if (std::isupper(pgn[0])) {
      m.promotion_piece_type_ =
          static_cast<promotion_piece_type>(to_piece_type(pgn[0]) - 1);
      ++pgn;
    }

    skip_stuff(pgn);

    return m;
  }
}

void skip_black_move_number(utl::cstr& pgn) {
  pgn = pgn.skip_whitespace_front();
  while (!pgn.empty() && (std::isdigit(pgn[0]) || pgn[0] == '.')) {
    ++pgn;
  }
  pgn = pgn.skip_whitespace_front();
}

std::vector<game::move> parse_moves(utl::cstr& pgn) {
  auto moves = std::vector<game::move>{};
  while (true) {
    if (is_result(pgn)) {
      break;
    }

    auto const move_number_end_pos = pgn.view().find(". ");
    utl::verify(move_number_end_pos != std::string_view::npos,
                "expected move number, got \"{}\"", shorten(pgn));
    pgn += move_number_end_pos + 2;

    auto const move_end_pos = pgn.view().find(' ');
    utl::verify(move_end_pos != std::string::npos,
                "movelist terminated without result");

    if (is_result(pgn)) {
      break;
    }
    moves.emplace_back(parse_move(pgn));

    if (is_result(pgn)) {
      break;
    }

    skip_black_move_number(pgn);

    moves.emplace_back(parse_move(pgn));
  }
  return moves;
}

game parse_pgn(utl::cstr& pgn) {
  auto g = game{};
  g.header_ = parse_header(pgn);
  g.moves_ = parse_moves(pgn);
  return g;
}

}  // namespace chessbot