#include <iostream>

#include "cista/mmap.h"

#include "chessbot/pgn.h"
#include "chessbot/position.h"
#include "chessbot/stockfish_evals.h"
#include "chessbot/util.h"

using namespace chessbot;

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cout << "usage: " << argv[0] << " PGN_FILE\n";
    return 1;
  }
  auto const path = argv[1];
  auto const min_moves = 25;
  auto const min_elo = 2000;
  auto const fen_count = std::numeric_limits<int>::max();
  auto const threshold = -15;
  auto const min_time = 3.0;

  auto m = cista::mmap{path, cista::mmap::protection::READ};
  auto pgn = utl::cstr{m.data(), m.size()};
  auto c = 0;
  while (!pgn.empty() && c != fen_count) {
    pgn = pgn.skip_whitespace_front();
    if (pgn.empty()) {
      std::cout << "END OF PGNs reached, written " << c
                << " FENs with evaluations\n";
      break;
    }
    auto const g = parse_pgn(pgn);

    try {
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
      auto max = std::max_element(begin(evals), end(evals));
      if (max->second.mate_ < 0 || max->second.cp_ < threshold) {
        continue;
      }
      std::cout << p.to_fen() << " ";
      for (auto const& [m, e] : evals) {
        std::cout << m << " " << (e.mate_ != 0 ? "M" : "")
                  << (e.mate_ != 0 ? e.mate_ : e.cp_) << " ";
      }
      std::cout << std::endl;

      ++c;
    } catch (std::exception const& e) {
      std::cerr << "PROBLEM WITH GAME " << g.header_.site_ << "\n";
      std::cerr << e.what() << "\n";
    }
  }
}
