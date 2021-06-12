#include "doctest/doctest.h"

#include <iomanip>
#include <iostream>

#include "chessbot/nn.h"
#include "chessbot/plot.h"
#include "chessbot/timing.h"

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

  auto const input = std::array{real_t{0.05}, real_t{0.1}};
  auto const expected = std::array{real_t{0.01}, real_t{0.99}};

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

TEST_CASE("nn adam update") {
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

  auto l1_orig = layer<2, 2>{};
  l1_orig.weights_[0][0] = 0.15F;
  l1_orig.weights_[0][1] = 0.20F;
  l1_orig.weights_[1][0] = 0.25F;
  l1_orig.weights_[1][1] = 0.30F;
  l1_orig.bias_weight_[0] = 0.35F;
  l1_orig.bias_weight_[1] = 0.35F;

  auto l2_orig = layer<2, 2>{};
  l2_orig.weights_[0][0] = 0.40F;
  l2_orig.weights_[0][1] = 0.45F;
  l2_orig.weights_[1][0] = 0.50F;
  l2_orig.weights_[1][1] = 0.55F;
  l2_orig.bias_weight_[0] = 0.60F;
  l2_orig.bias_weight_[1] = 0.60F;

  network<2, 2, 2> n_adam_epoch;
  auto& [l1_adam_epoch, l2_adam_epoch] = n_adam_epoch.layers_;

  l1_adam_epoch.weights_[0][0] = 0.15F;
  l1_adam_epoch.weights_[0][1] = 0.20F;
  l1_adam_epoch.weights_[1][0] = 0.25F;
  l1_adam_epoch.weights_[1][1] = 0.30F;
  l1_adam_epoch.bias_weight_[0] = 0.35F;
  l1_adam_epoch.bias_weight_[1] = 0.35F;

  l2_adam_epoch.weights_[0][0] = 0.40F;
  l2_adam_epoch.weights_[0][1] = 0.45F;
  l2_adam_epoch.weights_[1][0] = 0.50F;
  l2_adam_epoch.weights_[1][1] = 0.55F;
  l2_adam_epoch.bias_weight_[0] = 0.60F;
  l2_adam_epoch.bias_weight_[1] = 0.60F;

  auto const input = std::array{real_t{0.05}, real_t{0.1}};
  auto const expected = std::array{real_t{0.01}, real_t{0.99}};

  auto const input_epoch = std::array<std::array<real_t, 2>, 1>{input};
  auto const expected_epoch = std::array<std::array<real_t, 2>, 1>{expected};

  auto l1_m = layer<2, 2>{};
  auto l2_m = layer<2, 2>{};
  auto l1_v = layer<2, 2>{};
  auto l2_v = layer<2, 2>{};

  constexpr auto const beta1 = 0.9;
  constexpr auto const beta2 = 0.999;
  constexpr auto const inner_loop_size = 1000;
  constexpr auto const alpha = 0.001;

  // TODO learn quadratic function
  // TODO parallel: normal update instead of adam to compare errors

  std::cout << "m for l1: " << l1_m << "\n";
  std::cout << "m for l2: " << l2_m << "\n";
  std::cout << "v for l1: " << l1_v << "\n";
  std::cout << "v for l2: " << l2_v << "\n";

  std::cout << "---- LOOP\n";
  auto pl_absolute_errors = plot{""};

  constexpr auto const loop_size = 100000;

  n_adam_epoch.train_epoch_adam(
      input_epoch, expected_epoch, alpha, loop_size, [&](unsigned const i) {
        auto const out = n_adam_epoch.estimate(input);
        auto const e0_orig = (out[0] - expected[0]);
        auto const e1_orig = (out[1] - expected[1]);
        pl_absolute_errors.add_entry(i, e0_orig * e0_orig + e1_orig * e1_orig,
                                     2);
      });

  for (auto i = 0U; i != loop_size; ++i) {
    auto const t = i + 1;

    auto const l1_net = l1.net(input);
    auto const l2_net = l2.net(l1.estimate(input));

    auto const l1_out = l1.estimate(input);
    auto const l2_out = l2.estimate(l1_out);

    auto const diff =
        std::array{expected[0] - l2_out[0], expected[1] - l2_out[1]};
    auto const l2_deltas = l2.deltas(diff, l2_out);
    auto const l1_deltas = l1.deltas(l2, l1_out, l2_deltas);

    l1_m.adam_assign_moment<false>(l1_deltas, input, beta1);
    l2_m.adam_assign_moment<false>(l2_deltas, l1_out, beta1);
    l1_v.adam_assign_moment<true>(l1_deltas, input, beta2);
    l2_v.adam_assign_moment<true>(l2_deltas, l1_out, beta2);
    l1.adam_update_weights(l1_m, l1_v, beta1, beta2, alpha, t);
    l2.adam_update_weights(l2_m, l2_v, beta1, beta2, alpha, t);

    std::cout << "t=" << t << "\n";
    std::cout << "m for l1: " << l1_m << "\n";
    std::cout << "m for l2: " << l2_m << "\n";
    std::cout << "v for l1: " << l1_v << "\n";
    std::cout << "v for l2: " << l2_v << "\n";

    auto const l1_out_1 = l1.estimate(input);
    auto const l2_out_1 = l2.estimate(l1_out);

    auto const e0 = (l2_out_1[0] - expected[0]);
    auto const e1 = (l2_out_1[1] - expected[1]);

    pl_absolute_errors.add_entry(i, e0 * e0 + e1 * e1, 0);

    // original

    auto const l1_net_orig = l1_orig.net(input);
    auto const l2_net_orig = l2_orig.net(l1_orig.estimate(input));

    auto const l1_out_orig = l1_orig.estimate(input);
    auto const l2_out_orig = l2_orig.estimate(l1_out_orig);

    auto const diff_orig =
        std::array{expected[0] - l2_out_orig[0], expected[1] - l2_out_orig[1]};
    auto const l2_deltas_orig = l2_orig.deltas(diff_orig, l2_out_orig);
    auto const l1_deltas_orig =
        l1_orig.deltas(l2_orig, l1_out_orig, l2_deltas_orig);
    l2_orig.update_weights(l2_deltas_orig, l1_out_orig, 0.5F);
    l1_orig.update_weights(l1_deltas_orig, input, 0.5F);

    auto const l1_out_1_orig = l1_orig.estimate(input);
    auto const l2_out_1_orig = l2_orig.estimate(l1_out_orig);

    auto const e0_orig = (l2_out_1_orig[0] - expected[0]);
    auto const e1_orig = (l2_out_1_orig[1] - expected[1]);

    pl_absolute_errors.add_entry(i, e0_orig * e0_orig + e1_orig * e1_orig, 1);
  }
  pl_absolute_errors.do_plot();

  auto const l1_out_1 = l1.estimate(input);
  auto const l2_out_1 = l2.estimate(l1_out_1);
  CHECK(l2_out_1[0] == doctest::Approx(0.01));
  CHECK(l2_out_1[1] == doctest::Approx(0.99));
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
  auto const input = std::array{real_t{0.05}, real_t{0.1}};
  auto const expected = std::array{real_t{0.01}, real_t{0.99}};
  auto const error = [&](auto const& out) {
    auto const diff_0 = (out[0] - expected[0]);
    auto const diff_1 = (out[1] - expected[1]);
    return 0.5 * ((diff_0 * diff_0) + (diff_1 * diff_1));
  };

  // only true if we skip the bias weight update
  //  CHECK(error(n.estimate(input)) == doctest::Approx(0.298371109));

  n.train(input, expected, learning_rate);

  CHECK(l2.weights_[0][0] == doctest::Approx(0.35891648));
  CHECK(l2.weights_[0][1] == doctest::Approx(0.408666186));
  CHECK(l2.weights_[1][0] == doctest::Approx(0.511301270));
  CHECK(l2.weights_[1][1] == doctest::Approx(0.561370121));

  CHECK(l1.weights_[0][0] == doctest::Approx(0.149780716));
  CHECK(l1.weights_[0][1] == doctest::Approx(0.19956143));
  CHECK(l1.weights_[1][0] == doctest::Approx(0.24975114));
  CHECK(l1.weights_[1][1] == doctest::Approx(0.29950229));

  // only true if we skip the bias weight update
  //  CHECK(error(n.estimate(input)) == doctest::Approx(0.291027924));

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
  auto const n = std::make_unique<network<2, 2, 2>>(0, 150.0);

  for (auto i = 0U; i < 50000; ++i) {
    auto const x = static_cast<real_t>(2.5 + 5.0 * static_cast<real_t>(rand()) /
                                                 (RAND_MAX));
    auto const in = std::array{x, x};
    auto const out = std::array{in[0] * in[0], in[0] * in[0]};
    for (auto j = 0U; j < 300; ++j) {
      n->train(in, out, 0.01);
    }
  }

  for (auto i = real_t{4.0}; i < real_t{6.0}; i += .1) {
    const auto d = n->estimate(std::array{i, i})[0];
    CHECK(std::abs(d - (i * i)) < 1);
  }
}

TEST_CASE("nn quadratic function (batch == online) for batch size 1") {
  srand(0);
  auto const n_batch = std::make_unique<network<2, 2, 2>>(0, 150.0);

  srand(0);
  auto const n_online = std::make_unique<network<2, 2, 2>>(0, 150.0);

  constexpr auto const batch_size = 1U;
  constexpr auto const train_loop_size = 250U;

  srand(0);
  auto online_timing_sum = uint64_t{};
  auto batch_timing_sum = uint64_t{};
  for (auto i = 0U; i < 5000; ++i) {
    auto out = std::array<std::array<real_t, 2>, batch_size>{};
    auto in = std::array<std::array<real_t, 2>, batch_size>{};

    for (auto j = 0U; j < batch_size; ++j) {
      auto const x = static_cast<real_t>(
          2.5 + 5.0 * static_cast<real_t>(rand()) / (RAND_MAX));
      in[j] = {x, x};
      out[j] = {x * x, x * x};
    }

    CHESSBOT_START_TIMING(online_timing);
    for (auto j = 0U; j < batch_size; ++j) {
      for (auto k = 0U; k < train_loop_size; ++k) {
        n_online->train(in[j], out[j], 0.1);
      }
    }
    CHESSBOT_STOP_TIMING(online_timing);
    online_timing_sum += CHESSBOT_TIMING_US(online_timing);

    CHESSBOT_START_TIMING(batch_timing);
    n_batch->train_epoch(in, out, 0.1, train_loop_size, [](unsigned) {});
    CHESSBOT_STOP_TIMING(batch_timing);
    batch_timing_sum += CHESSBOT_TIMING_US(batch_timing);
  }

  for (auto i = real_t{4.0}; i < real_t{6.0}; i += .1) {
    const auto d1 = n_batch->estimate(std::array{i, i})[0];
    const auto d2 = n_online->estimate(std::array{i, i})[0];
    CHECK(d1 == doctest::Approx(d2));
  }

  std::cout << "batch: " << (batch_timing_sum / 1000.0) << "ms\n";
  std::cout << "online: " << (online_timing_sum / 1000.0) << "ms\n";
}

TEST_CASE("nn test epoch") {
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
  auto input = std::array<std::array<real_t, 2>, 2>{};
  input[0] = {0.05, 0.1};
  input[1] = {0.05, 0.1};
  auto expected = std::array<std::array<real_t, 2>, 2>{};
  expected[0] = {0.01, 0.99};
  expected[1] = {0.01, 0.99};
  auto const error = [&](auto const& out) {
    auto const diff_0 = (out[0] - expected[0][0]);
    auto const diff_1 = (out[1] - expected[0][1]);
    return 0.5 * ((diff_0 * diff_0) + (diff_1 * diff_1));
  };

  CHECK(error(n.estimate(input[0])) == doctest::Approx(0.298371109));

  n.train_epoch(input, expected, learning_rate, 1, [](unsigned) {});

  CHECK(l2.weights_[0][0] == doctest::Approx(0.35891648));
  CHECK(l2.weights_[0][1] == doctest::Approx(0.408666186));
  CHECK(l2.weights_[1][0] == doctest::Approx(0.511301270));
  CHECK(l2.weights_[1][1] == doctest::Approx(0.561370121));

  CHECK(l1.weights_[0][0] == doctest::Approx(0.149780716));
  CHECK(l1.weights_[0][1] == doctest::Approx(0.19956143));
  CHECK(l1.weights_[1][0] == doctest::Approx(0.24975114));
  CHECK(l1.weights_[1][1] == doctest::Approx(0.29950229));

  // only true if we skip the bias weight update
  //  CHECK(error(n.estimate(input[0])) == doctest::Approx(0.291027924));

  n.train_epoch(input, expected, learning_rate, 10000, [](unsigned) {});
  auto const out = n.estimate(input[0]);

  auto const diff_0 = (out[0] - expected[0][0]);
  auto const diff_1 = (out[1] - expected[0][1]);
  auto const err = error(out);

  CHECK(err < 0.0001);
}
