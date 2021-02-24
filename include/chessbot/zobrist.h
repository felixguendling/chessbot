#pragma once

#include <cinttypes>
#include <array>

namespace chessbot {

struct position;

using zobrist_t = uint64_t;

enum castling_right { WHITE_SHORT, WHITE_LONG, BLACK_SHORT, BLACK_LONG };

extern std::array<std::array<zobrist_t, 2>, 64> zobrist_color_hashes;
extern std::array<std::array<zobrist_t, 6>, 64> zobrist_piece_hashes;
extern std::array<zobrist_t, 4> zobrist_castling_right_hashes;
extern std::array<zobrist_t, 8> zobrist_en_passant_hashes;

zobrist_t compute_hash(position const&);

}  // namespace chessbot