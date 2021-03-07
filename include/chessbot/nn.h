#pragma once

#include <cmath>
#include <array>
#include <tuple>

namespace chessbot {

using real_t = double;

inline real_t activation_fn(real_t const t) {
  return 1.0F / (1.0F + std::exp(-t));
}

inline real_t activation_fn_d(real_t const x) { return x * (1 - x); }

// inline real_t activation_fn(real_t const t) { return std::max(0.0F, t); }
// inline real_t activation_fn_d(real_t const t) { return t < 0 ? 0 : 1; }

template <unsigned InputSize, unsigned LayerSize>
struct layer {
  static constexpr auto const layer_size = LayerSize;

  layer() {
    for (auto& x : weights_) {
      for (auto& y : x) {
        y = -1.0 + 2 * ((double)rand() / (RAND_MAX));
        //        y = 0.5;
      }
    }
    for (auto& b : bias_weight_) {
      b = -1.0 + 2 * ((double)rand() / (RAND_MAX));
      //      b = 0.5;
    }
  }

  std::array<real_t, LayerSize> net(
      std::array<real_t, InputSize> const& input) const {
    auto output = std::array<real_t, LayerSize>{};
    for (auto i = 0; i < LayerSize; ++i) {
      for (auto j = 0; j < input.size(); ++j) {
        output[i] += input[j] * weights_[i][j];
      }
      output[i] += bias_weight_[i] * 1.0F;
    }
    return output;
  }

  std::array<real_t, LayerSize> estimate(
      std::array<real_t, InputSize> const& input) const {
    auto n = net(input);
    for (auto& x : n) {
      x = activation_fn(x);
    }
    return n;
  }

  template <typename NextLayer>
  std::array<real_t, LayerSize> deltas(
      NextLayer const& next, std::array<real_t, LayerSize> const& out,
      std::array<real_t, NextLayer::layer_size> const& delta) const {
    auto deltas = std::array<real_t, LayerSize>{};
    for (auto j = 0U; j < LayerSize; ++j) {
      auto sum = 0.0F;
      for (auto k = 0U; k < deltas.size(); ++k) {
        sum += delta[k] * next.weights_[k][j];
      }
      deltas[j] = activation_fn_d(out[j]) * sum;
    }
    return deltas;
  }

  std::array<real_t, LayerSize> deltas(
      std::array<real_t, LayerSize> const& diff,
      std::array<real_t, LayerSize> const& out) const {
    auto deltas = std::array<real_t, LayerSize>{};
    for (auto i = 0U; i < diff.size(); ++i) {
      deltas[i] = -diff[i] * activation_fn_d(out[i]);
    }
    return deltas;
  }

  void update_weights(std::array<real_t, LayerSize> const& deltas,
                      std::array<real_t, InputSize> const& prev_layer_out,
                      real_t const learning_rate) {
    for (auto i = 0U; i < InputSize; ++i) {
      for (auto j = 0U; j < LayerSize; ++j) {
        weights_[i][j] += (-learning_rate) * deltas[i] * prev_layer_out[j];
      }
    }
  }

  std::array<std::array<real_t, InputSize>, LayerSize> weights_;
  std::array<real_t, LayerSize> bias_weight_;
};

template <unsigned InputSize, unsigned... LayerSizes>
struct network {
  using deltas_t = std::tuple<std::array<real_t, LayerSizes>...>;
  using layer_outputs_t = std::tuple<std::array<real_t, InputSize>,
                                     std::array<real_t, LayerSizes>...>;

  static constexpr auto const layer_sizes =
      std::array<unsigned, sizeof...(LayerSizes)>{LayerSizes...};

  static constexpr auto const number_of_layers = sizeof...(LayerSizes);

  static constexpr std::size_t get_layer_size(std::size_t const i) {
    return layer_sizes[i];
  }

  static constexpr std::size_t get_input_size(std::size_t const i) {
    if (i == 0) {
      return InputSize;
    } else {
      return layer_sizes[i - 1];
    }
  }

  template <std::size_t... Is>
  static constexpr auto get_type_helper(std::index_sequence<Is...>) {
    return std::tuple<layer<get_input_size(Is), get_layer_size(Is)>...>{};
  }

  template <typename Indices = std::make_index_sequence<sizeof...(LayerSizes)>>
  static constexpr auto get_type() {
    return get_type_helper(Indices{});
  }

  using layers_tuple_t = decltype(get_type());
  using input_t = std::array<real_t, InputSize>;
  using output_t = std::array<real_t, layer_sizes.back()>;

  explicit network(real_t const min = 0.0, real_t const max = 1.0)
      : min_{min}, max_{max} {}

  real_t scale_to_output(real_t const v) { return min_ + v * (max_ - min_); }

  real_t scale_from_output(real_t const v) {
    return (v - min_) / (max_ - min_);
  }

  template <size_t I, typename std::enable_if_t<I == 0>* = nullptr>
  auto e(input_t const& in) {
    return std::get<0>(layers_).estimate(in);
  }

  template <size_t I, typename std::enable_if_t<I != 0>* = nullptr>
  auto e(input_t const& in) {
    return std::get<I>(layers_).estimate(e<I - 1>(in));
  }

  output_t estimate(input_t const& in) {
    auto scaled_input = in;
    for (auto& s : scaled_input) {
      s = scale_from_output(s);
    }

    auto result = e<number_of_layers - 1>(scaled_input);
    for (auto& r : result) {
      r = scale_to_output(r);
    }
    return result;
  }

  template <size_t I,
            typename std::enable_if_t<I == number_of_layers>* = nullptr>
  void compute_outputs(layer_outputs_t&) {}

  template <size_t I,
            typename std::enable_if_t<I != number_of_layers>* = nullptr>
  void compute_outputs(layer_outputs_t& out) {
    std::get<I + 1>(out) = std::get<I>(layers_).estimate(std::get<I>(out));
    compute_outputs<I + 1>(out);
  }

  template <size_t I, typename std::enable_if_t<I == 0>* = nullptr>
  void compute_deltas(layer_outputs_t const& outs, deltas_t& deltas) {
    std::get<I>(deltas) = std::get<I>(layers_).deltas(std::get<I + 1>(layers_),
                                                      std::get<I + 1>(outs),
                                                      std::get<I + 1>(deltas));
  }

  template <size_t I, typename std::enable_if_t<I != 0>* = nullptr>
  void compute_deltas(layer_outputs_t const& outs, deltas_t& deltas) {
    std::get<I>(deltas) = std::get<I>(layers_).deltas(std::get<I + 1>(layers_),
                                                      std::get<I + 1>(outs),
                                                      std::get<I + 1>(deltas));
    compute_deltas<I - 1>(outs, deltas);
  }

  template <size_t I, typename std::enable_if_t<I == 0>* = nullptr>
  void update_weights(layer_outputs_t const& outs, deltas_t const& deltas,
                      real_t const learning_rate) {
    std::get<I>(layers_).update_weights(std::get<I>(deltas), std::get<I>(outs),
                                        learning_rate);
  }

  template <size_t I, typename std::enable_if_t<I != 0>* = nullptr>
  void update_weights(layer_outputs_t const& outs, deltas_t const& deltas,
                      real_t const learning_rate) {
    std::get<I>(layers_).update_weights(std::get<I>(deltas), std::get<I>(outs),
                                        learning_rate);
    update_weights<I - 1>(outs, deltas, learning_rate);
  }

  void train(input_t const& in, output_t const& expected,
             real_t const learning_rate) {
    auto outs = layer_outputs_t{};
    std::get<0>(outs) = in;
    for (auto& s : std::get<0>(outs)) {
      s = scale_from_output(s);
    }

    auto scaled_expected = expected;
    for (auto& s : scaled_expected) {
      s = scale_from_output(s);
    }

    compute_outputs<0>(outs);

    auto diff = output_t{};
    auto& last_layer_out = std::get<number_of_layers>(outs);
    for (auto i = 0U; i < last_layer_out.size(); ++i) {
      diff[i] = scaled_expected[i] - last_layer_out[i];
    }

    auto deltas = deltas_t{};
    auto& last_layer = std::get<number_of_layers - 1>(layers_);
    auto& last_layer_deltas = std::get<number_of_layers - 1>(deltas);
    last_layer_deltas = last_layer.deltas(diff, last_layer_out);
    compute_deltas<number_of_layers - 2>(outs, deltas);
    update_weights<number_of_layers - 1>(outs, deltas, learning_rate);
  }

  /*
  output_t estimate1(input_t const& in) {
    auto scaled_input = in;
    for (auto& s : scaled_input) {
      s = scale_from_output(s);
    }

    auto& l1 = std::get<0>(layers_);
    auto& l2 = std::get<1>(layers_);
    auto result = l2.estimate(l1.estimate(scaled_input));
    for (auto& r : result) {
      r = scale_to_output(r);
    }
    return result;
  }

  void train1(input_t const& in, output_t const& expected,
              real_t const learning_rate) {
    auto scaled_input = in;
    for (auto& s : scaled_input) {
      s = scale_from_output(s);
    }

    auto scaled_expected = expected;
    for (auto& s : scaled_expected) {
      s = scale_from_output(s);
    }

    auto& l1 = std::get<0>(layers_);
    auto& l2 = std::get<1>(layers_);

    auto const l1_out = l1.estimate(scaled_input);
    auto const l2_out = l2.estimate(l1_out);

    auto diff = std::array<real_t, 2>{};
    for (auto i = 0U; i < l2_out.size(); ++i) {
      diff[i] = scaled_expected[i] - l2_out[i];
    }

    auto const l2_deltas = l2.deltas(diff, l2_out);
    auto const l1_deltas = l1.deltas(l2, l1_out, l2_deltas);

    l2.update_weights(l2_deltas, l1_out, learning_rate);
    l1.update_weights(l1_deltas, scaled_input, learning_rate);
  }
  */

  real_t min_, max_;
  layers_tuple_t layers_;
};

}  // namespace chessbot