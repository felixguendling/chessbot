#pragma once

#include <cinttypes>
#include <array>

namespace chessbot {

using zobrist_t = uint64_t;

extern std::array<std::array<std::array<zobrist_t, 6>, 2>, 64>
    zobrist_piece_hashes;

extern std::array<zobrist_t, 4> zobrist_castling_rights_hashes;

extern std::array<zobrist_t, 8> en_passant_hashes;

}  // namespace chessbot