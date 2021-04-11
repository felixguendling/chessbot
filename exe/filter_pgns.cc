#include <iostream>

#include "cista/mmap.h"

#include "chessbot/pgn.h"
#include "chessbot/position.h"
#include "chessbot/stockfish_evals.h"
#include "chessbot/util.h"

using namespace chessbot;

int main() {
  auto const path =
      "/home/felix/Downloads/lichess_db_standard_rated_2017-04.pgn";
  auto const min_moves = 25;
  auto const min_elo = 2000;
  auto const fen_count = 10;
  auto const threshold = 0.1;
  auto const min_time = 3.0;

  auto m = cista::mmap{path, cista::mmap::protection::READ};
  auto pgn = utl::cstr{m.data(), m.size()};
  auto c = 0;
  while (!pgn.empty() && c != fen_count) {
    pgn = pgn.skip_whitespace_front();
    if (pgn.empty()) {
      break;
    }
    auto const g = parse_pgn(pgn);
    if (g.header_.start_time_ +
                g.header_.time_increment_ * g.moves_.size() / 60.0 <
            min_time ||
        g.header_.elo_black_ < min_elo || g.header_.elo_white_ < min_elo ||
        g.moves_.size() < min_moves) {
      continue;
    }

    auto const r = get_random_number() % g.moves_.size();
    auto p = position::from_fen(start_position_fen);
    for (auto i = 0; i < r; ++i) {
      p.make_pgn_move(g.moves_[i], nullptr);
    }
    auto const evals = stockfish_evals(p);
    auto max = std::max_element(
        begin(evals), end(evals),
        [](auto const& a, auto const& b) { return a.second < b.second; });
    if (max->second < threshold) {
      continue;
    }
    std::cout << p.to_fen() << " ";
    for (auto const& [m, e] : evals) {
      std::cout << m << " " << e << " ";
    }
    std::cout << std::endl;

    //    if (c % 100 == 0) {
    //      std::cout << "\r" << c << std::flush;
    //    }
    ++c;
  }
  //  std::cout << "\n" << c << "\n";
}