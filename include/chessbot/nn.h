#pragma once

#include <cmath>
#include <array>
#include <tuple>

#include "chessbot/optimizer.h"
#include "chessbot/real_t.h"
#include "chessbot/sigmoid.h"

namespace chessbot {

// ### TANH ###
// inline real_t activation_fn(real_t const t) { return std::tanh(t); }
// inline real_t activation_fn_d(real_t const x) { return 1 - (x * x); }
// inline real_t scale_to_output(real_t const min, real_t const max,
//                              real_t const v) {
//  return min + ((1.0 + v) / 2.0) * (max - min);
//}
// inline real_t scale_from_output(real_t const min, real_t const max,
//                                real_t const v) {
//  return -1.0 + 2 * ((v - min) / (max - min));
//}

// ### SIGMOID ###
inline real_t activation_fn(real_t const t) { return sigmoid(t); }
inline real_t activation_fn_d(real_t const x) { return x * (1 - x); }
inline real_t scale_to_output(real_t const min, real_t const max,
                              real_t const v) {
  return min + v * (max - min);
}
inline real_t scale_from_output(real_t const min, real_t const max,
                                real_t const v) {
  return (v - min) / (max - min);
}

// inline real_t activation_fn(real_t const t) { return std::max(0.0, t); }
// inline real_t activation_fn_d(real_t const t) { return t <= 0 ? 0 : 1; }

template <unsigned InputSize, unsigned LayerSize>
struct layer {
  static constexpr auto const input_size = InputSize;
  static constexpr auto const layer_size = LayerSize;

  layer() = default;

  friend std::ostream& operator<<(std::ostream& out, layer const& l) {
    for (auto const& m : l.weights_) {
      for (auto const& x : m) {
        out << x << " ";
      }
    }
    return out;
  }

  void init_random() {
    for (auto& x : weights_) {
      for (auto& y : x) {
        auto const z = 1.0 / std::sqrt(InputSize);
        y = -z + 2 * z * ((double)rand() / (RAND_MAX));
        // TODO for ReLU: only positive values?
      }
    }
    for (auto& b : bias_weight_) {
      b = 0.0;
    }
  }

  layer& operator+=(layer const& o) {
    for (auto i = 0U; i != weights_.size(); ++i) {
      for (auto j = 0U; j != weights_[i].size(); ++j) {
        weights_[i][j] += o.weights_[i][j];
      }
    }
    for (auto i = 0U; i != bias_weight_.size(); ++i) {
      bias_weight_[i] += o.bias_weight_[i];
    }
    return *this;
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
      std::array<real_t, NextLayer::layer_size> const& next_layer_deltas)
      const {
    auto deltas = std::array<real_t, LayerSize>{};
    for (auto j = 0U; j < LayerSize; ++j) {
      auto sum = 0.0F;
      for (auto k = 0U; k < next_layer_deltas.size(); ++k) {
        sum += next_layer_deltas[k] * next.weights_[k][j];
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
    for (auto i = 0U; i < LayerSize; ++i) {
      for (auto j = 0U; j < InputSize; ++j) {
        weights_[i][j] += (-learning_rate) * deltas[i] * prev_layer_out[j];
      }
    }
    for (auto i = 0U; i < LayerSize; ++i) {
      bias_weight_[i] += (-learning_rate) * deltas[i];
    }
  }

  std::array<std::array<real_t, InputSize>, LayerSize> weights_{};
  std::array<real_t, LayerSize> bias_weight_{};
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
      : min_{min}, max_{max} {
    init_random();
  }

  template <size_t I = number_of_layers - 1>
  void init_random() {
    std::get<I>(layers_).init_random();
    if constexpr (I != 0U) {
      init_random<I - 1>();
    }
  }

  template <size_t I, typename std::enable_if_t<I == 0>* = nullptr>
  auto e(input_t const& in) const {
    return std::get<0>(layers_).estimate(in);
  }

  template <size_t I, typename std::enable_if_t<I != 0>* = nullptr>
  auto e(input_t const& in) const {
    return std::get<I>(layers_).estimate(e<I - 1>(in));
  }

  output_t estimate(input_t const& in) const {
    auto scaled_input = in;
    for (auto& s : scaled_input) {
      s = scale_from_output(min_, max_, s);
    }

    auto result = e<number_of_layers - 1>(scaled_input);
    for (auto& r : result) {
      r = scale_to_output(min_, max_, r);
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
  void update_weights(layers_tuple_t& layers, layer_outputs_t const& outs,
                      deltas_t const& deltas, real_t const learning_rate) {
    std::get<I>(layers).update_weights(std::get<I>(deltas), std::get<I>(outs),
                                       learning_rate);
  }

  template <size_t I, typename std::enable_if_t<I != 0>* = nullptr>
  void update_weights(layers_tuple_t& layers, layer_outputs_t const& outs,
                      deltas_t const& deltas, real_t const learning_rate) {
    std::get<I>(layers).update_weights(std::get<I>(deltas), std::get<I>(outs),
                                       learning_rate);
    update_weights<I - 1>(layers, outs, deltas, learning_rate);
  }

  template <size_t BatchSize, size_t I = number_of_layers - 1>
  void divide_by_batch_size(layers_tuple_t& sum) {
    auto& l = std::get<I>(sum);
    for (auto i = 0; i < l.weights_.size(); ++i) {
      for (auto j = 0; j < l.weights_[i].size(); ++j) {
        l.weights_[i][j] /= BatchSize;
      }
      l.bias_weight_[i] /= BatchSize;
    }
    if constexpr (I != 0) {
      divide_by_batch_size<BatchSize, I - 1>(sum);
    }
  }

  template <size_t I = number_of_layers - 1>
  void zero_out(layers_tuple_t& sum) {
    auto& l = std::get<I>(sum);
    for (auto i = 0; i < l.weights_.size(); ++i) {
      for (auto j = 0; j < l.weights_[i].size(); ++j) {
        l.weights_[i][j] = 0;
      }
      l.bias_weight_[i] = 0;
    }
    if constexpr (I != 0) {
      zero_out<I - 1>(sum);
    }
  }

  template <size_t I = number_of_layers - 1>
  void add(layers_tuple_t& sum, layers_tuple_t const& copy) {
    std::get<I>(sum) += std::get<I>(copy);
    if constexpr (I != 0U) {
      add<I - 1>(sum, copy);
    }
  }

  template <template <typename> typename Optimizer = sgd, size_t BatchSize,
            typename PlotFn>
  void train_epoch(std::array<input_t, BatchSize> const& in,
                   std::array<output_t, BatchSize> const& expected,
                   real_t const learning_rate, unsigned const outer_loop_size,
                   PlotFn&& plot) {
    auto optimizer = Optimizer<layers_tuple_t>{};
    for (auto i = 0; i != outer_loop_size; i++) {
      zero_out(sum_);
      for (auto batch_idx = 0; batch_idx < BatchSize; ++batch_idx) {
        copy_ = layers_;
        train(sum_, in[batch_idx], expected[batch_idx], -1);
      }
      divide_by_batch_size<BatchSize>(sum_);
      optimizer.update(sum_, layers_);
      plot(i);
    }
  }

  void train(layers_tuple_t& sum_layers, input_t const& in,
             output_t const& expected, real_t const learning_rate) {
    auto outs = layer_outputs_t{};
    std::get<0>(outs) = in;
    for (auto& s : std::get<0>(outs)) {
      s = scale_from_output(min_, max_, s);
    }

    auto scaled_expected = expected;
    for (auto& s : scaled_expected) {
      s = scale_from_output(min_, max_, s);
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
    update_weights<number_of_layers - 1>(sum_layers, outs, deltas,
                                         learning_rate);
  }

  void train(input_t const& in, output_t const& expected,
             real_t const learning_rate) {
    train(layers_, in, expected, learning_rate);
  }

  real_t min_, max_;
  layers_tuple_t layers_{}, copy_{}, sum_{};
};

}  // namespace chessbot
