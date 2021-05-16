#pragma once

#include <iosfwd>
#include <vector>

#include "utl/parser/cstr.h"

#include "chessbot/constants.h"
#include "chessbot/move.h"
#include "chessbot/square.h"

namespace chessbot {

struct game {
  struct header {
    std::string site_;
    static constexpr auto const infinite_time =
        std::numeric_limits<unsigned>::max();
    unsigned elo_white_{0U}, elo_black_{0U};
    unsigned start_time_{0U}, time_increment_{0U};
  } header_;

  struct move {
    static constexpr auto const no_promotion =
        promotion_piece_type{std::numeric_limits<
            std::underlying_type_t<promotion_piece_type>>::max()};
    static constexpr auto const unknown = std::numeric_limits<char>::max();

    friend std::ostream& operator<<(std::ostream&, move const&);

    piece_type piece_;
    char from_rank_{unknown}, from_file_{unknown};
    square_t to_{0};
    promotion_piece_type promotion_piece_type_{no_promotion};
    enum class castle { NONE, LONG, SHORT } castle_{castle::NONE};
  };

  std::vector<move> moves_;
};

game parse_pgn(utl::cstr&);
game::header parse_header(utl::cstr&);
game::move parse_move(utl::cstr&);
std::vector<game::move> parse_moves(utl::cstr&);

}  // namespace chessbot