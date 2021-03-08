#include "doctest/doctest.h"

#include <iomanip>
#include <iostream>

#include "chessbot/nn.h"

using namespace chessbot;

TEST_CASE("nn simple") {
  auto l1 = layer<2, 2>{};
  l1.weights_[0][0] = 0.15F;
  l1.weights_[0][1] = 0.20F;
  l1.weights_[1][0] = 0.25F;
  l1.weights_[1][1] = 0.30F;
  l1.bias_weight_[0] = 0.35F;
  l1.bias_weight_[1] = 0.35F;

  auto l2 = layer<2, 2>{};
  l2.weights_[0][0] = 0.40F;
  l2.weights_[0][1] = 0.45F;
  l2.weights_[1][0] = 0.50F;
  l2.weights_[1][1] = 0.55F;
  l2.bias_weight_[0] = 0.60F;
  l2.bias_weight_[1] = 0.60F;

  auto const input = std::array{0.05, 0.1};
  auto const expected = std::array{0.01, 0.99};

  auto const l1_net = l1.net(input);
  auto const l2_net = l2.net(l1.estimate(input));

  auto const l1_out = l1.estimate(input);
  auto const l2_out = l2.estimate(l1_out);

  auto const diff =
      std::array{expected[0] - l2_out[0], expected[1] - l2_out[1]};
  auto const l2_deltas = l2.deltas(diff, l2_out);
  auto const l1_deltas = l1.deltas(l2, l1_out, l2_deltas);
  l2.update_weights(l2_deltas, l1_out, 0.5F);
  l1.update_weights(l1_deltas, input, 0.5F);

  CHECK(l1_net[0] == doctest::Approx(0.37749));
  CHECK(l2_net[0] == doctest::Approx(1.10590));
  CHECK(l1_out[0] == doctest::Approx(0.59326));
  CHECK(l2_out[0] == doctest::Approx(0.75136));
  CHECK(l2_deltas[0] == doctest::Approx(0.13849));
  CHECK(l1_deltas[0] == doctest::Approx(0.00877));

  CHECK(l1_net[1] == doctest::Approx(0.39249));
  CHECK(l2_net[1] == doctest::Approx(1.22492));
  CHECK(l1_out[1] == doctest::Approx(0.59688));
  CHECK(l2_out[1] == doctest::Approx(0.77292));
  CHECK(l2_deltas[1] == doctest::Approx(-0.038098));
  CHECK(l1_deltas[1] == doctest::Approx(0.009954));

  CHECK(l2.weights_[0][0] == doctest::Approx(0.35891648));
  CHECK(l2.weights_[0][1] == doctest::Approx(0.408666186));
  CHECK(l2.weights_[1][0] == doctest::Approx(0.511301270));
  CHECK(l2.weights_[1][1] == doctest::Approx(0.561370121));

  CHECK(l1.weights_[0][0] == doctest::Approx(0.149780716));
  CHECK(l1.weights_[0][1] == doctest::Approx(0.19956143));
  CHECK(l1.weights_[1][0] == doctest::Approx(0.24975114));
  CHECK(l1.weights_[1][1] == doctest::Approx(0.29950229));
}

TEST_CASE("nn test") {
  network<2, 2, 2> n;
  auto& [l1, l2] = n.layers_;

  l1.weights_[0][0] = 0.15F;
  l1.weights_[0][1] = 0.20F;
  l1.weights_[1][0] = 0.25F;
  l1.weights_[1][1] = 0.30F;
  l1.bias_weight_[0] = 0.35F;
  l1.bias_weight_[1] = 0.35F;

  l2.weights_[0][0] = 0.40F;
  l2.weights_[0][1] = 0.45F;
  l2.weights_[1][0] = 0.50F;
  l2.weights_[1][1] = 0.55F;
  l2.bias_weight_[0] = 0.60F;
  l2.bias_weight_[1] = 0.60F;

  auto const learning_rate = 0.5;
  auto const input = std::array{0.05, 0.1};
  auto const expected = std::array{0.01, 0.99};
  auto const error = [&](auto const& out) {
    auto const diff_0 = (out[0] - expected[0]);
    auto const diff_1 = (out[1] - expected[1]);
    return 0.5 * ((diff_0 * diff_0) + (diff_1 * diff_1));
  };

  CHECK(error(n.estimate(input)) == doctest::Approx(0.298371109));

  n.train(input, expected, learning_rate);

  CHECK(l2.weights_[0][0] == doctest::Approx(0.35891648));
  CHECK(l2.weights_[0][1] == doctest::Approx(0.408666186));
  CHECK(l2.weights_[1][0] == doctest::Approx(0.511301270));
  CHECK(l2.weights_[1][1] == doctest::Approx(0.561370121));

  CHECK(l1.weights_[0][0] == doctest::Approx(0.149780716));
  CHECK(l1.weights_[0][1] == doctest::Approx(0.19956143));
  CHECK(l1.weights_[1][0] == doctest::Approx(0.24975114));
  CHECK(l1.weights_[1][1] == doctest::Approx(0.29950229));

  CHECK(error(n.estimate(input)) == doctest::Approx(0.291027924));

  auto err = real_t{};
  for (auto i = 0; i < 10000; ++i) {
    n.train(input, expected, learning_rate);
    auto const out = n.estimate(input);

    auto const diff_0 = (out[0] - expected[0]);
    auto const diff_1 = (out[1] - expected[1]);
    err = error(out);
  }

  CHECK(err < 0.0001);
}

TEST_CASE("nn quadratic function") {
  network<2, 2, 2> n{0, 300.0};

  for (auto i = 0U; i < 15000; ++i) {
    auto const x = 2.5 + 5.0 * static_cast<real_t>(rand()) / (RAND_MAX);
    auto const in = std::array{x, x};
    auto const out = std::array{in[0] * in[0], in[0] * in[0]};
    for (auto j = 0U; j < 600; ++j) {
      n.train(in, out, 0.5);
    }
  }

  for (auto i = 4.0; i < 6.0; i += .1) {
    const auto d = n.estimate(std::array{i, i})[0];
    CHECK(std::abs(d - (i * i)) < 0.44);
  }
}
