#include "chessbot/stockfish_evals.h"

#include "doctest/doctest.h"

using namespace chessbot;

TEST_CASE("move eval comparison") {
  SUBCASE("mate vs cp") {
    auto const e1 = move_eval{.mate_ = -2, .cp_ = 0};
    auto const e2 = move_eval{.mate_ = 0, .cp_ = 100};

    CHECK(e1 < e2);
  }

  SUBCASE("mate vs mate negative") {
    auto const e1 = move_eval{.mate_ = -2, .cp_ = 0};
    auto const e2 = move_eval{.mate_ = -3, .cp_ = 0};

    CHECK(!(e1 < e2));
    CHECK(e2 < e1);
  }

  SUBCASE("mate vs mate positive") {
    auto const e1 = move_eval{.mate_ = 2, .cp_ = 0};
    auto const e2 = move_eval{.mate_ = 3, .cp_ = 0};

    CHECK(!(e1 < e2));
    CHECK(e2 < e1);
  }

  SUBCASE("cp vs cp") {
    auto const e1 = move_eval{.mate_ = 0, .cp_ = 100};
    auto const e2 = move_eval{.mate_ = 0, .cp_ = 50};

    CHECK(!(e1 < e2));
    CHECK(e2 < e1);
  }

  SUBCASE("cp vs cp") {
    auto const e1 = move_eval{.mate_ = 0, .cp_ = 50};
    auto const e2 = move_eval{.mate_ = 0, .cp_ = 0};

    CHECK(!(e1 < e2));
    CHECK(e2 < e1);
  }

  SUBCASE("mate vs mate pos vs neg") {
    auto const e1 = move_eval{.mate_ = 5, .cp_ = 0};
    auto const e2 = move_eval{.mate_ = -5, .cp_ = 0};

    CHECK(!(e1 < e2));
    CHECK(e2 < e1);
  }
}
