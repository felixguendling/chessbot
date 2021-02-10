#include "doctest/doctest.h"

#include "chessbot/util.h"

TEST_CASE("square name test") { CHECK(chessbot::get_square_name(1U) == "a8"); }