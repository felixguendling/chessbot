#pragma once

#include <utility>

#include "chessbot/real_t.h"

namespace chessbot {

template <typename Layers>
struct adam {
  void update(Layers const& gradient, Layers& nn) {
    ++t_;
    constexpr auto const indices =
        std::make_index_sequence<number_of_layers_>();
    update_moments<false>(gradient, m_, beta1_, indices);
    update_moments<true>(gradient, v_, beta2_, indices);
    update_weights(nn, indices);
  }

  template <bool Square, std::size_t... I>
  void update_moments(Layers const& gradient, Layers& moment, real_t const beta,
                      std::index_sequence<I...>) {
    (assign_moment_layer<Square>(std::get<I>(moment), std::get<I>(gradient),
                                 beta),
     ...);
  }

  template <std::size_t... I>
  void update_weights(Layers& nn, std::index_sequence<I...>) {
    (update_weights_layer(std::get<I>(nn), std::get<I>(m_), std::get<I>(v_)),
     ...);
  }

  template <bool Square, typename Layer>
  void assign_moment_layer(Layer& moment, Layer const& gradient,
                           real_t const beta) {
    for (auto i = 0U; i < Layer::layer_size; ++i) {
      for (auto j = 0U; j < Layer::input_size; ++j) {
        moment.weights_[i][j] = beta * moment.weights_[i][j] +
                                (1 - beta) * (gradient.weights_[i][j]) *
                                    (Square ? (gradient.weights_[i][j]) : 1);
      }
    }
    for (auto i = 0U; i < Layer::layer_size; ++i) {
      moment.bias_weight_[i] = beta * moment.bias_weight_[i] +
                               (1 - beta) * gradient.bias_weight_[i] *
                                   (Square ? (gradient.bias_weight_[i]) : 1);
    }
  }

  template <typename Layer>
  void update_weights_layer(Layer& layer, Layer const& m, Layer const& v) {
    for (auto i = 0U; i < Layer::layer_size; ++i) {
      for (auto j = 0U; j < Layer::input_size; ++j) {
        layer.weights_[i][j] -=
            alpha_ * (m.weights_[i][j] / (1 - std::pow(beta1_, t_))) /
            (std::sqrt(v.weights_[i][j] / (1 - std::pow(beta2_, t_))) + 1E-8);
      }
    }
    for (auto i = 0U; i < Layer::layer_size; ++i) {
      layer.bias_weight_[i] -=
          alpha_ * (m.bias_weight_[i] / (1 - std::pow(beta1_, t_))) /
          (std::sqrt((v.bias_weight_[i] / (1 - std::pow(beta2_, t_)))) + 1E-8);
    }
  }

  static constexpr auto const beta1_ = 0.9;
  static constexpr auto const beta2_ = 0.99;
  static constexpr auto const alpha_ = 0.01;
  static constexpr auto const number_of_layers_ = std::tuple_size<Layers>();
  Layers m_{};
  Layers v_{};
  real_t t_{0.0};
};

template <typename Layers>
struct sgd {
  void update(Layers const& gradient, Layers& nn) {
    constexpr auto const indices =
        std::make_index_sequence<number_of_layers_>();
    update_weights(nn, gradient, indices);
  }

  template <std::size_t... I>
  void update_weights(Layers& nn, Layers const& gradient,
                      std::index_sequence<I...>) {
    (update_weights_layer(std::get<I>(nn), std::get<I>(gradient)), ...);
  }

  template <typename Layer>
  void update_weights_layer(Layer& layer, Layer const& gradient_layer) {
    for (auto i = 0U; i < Layer::layer_size; ++i) {
      for (auto j = 0U; j < Layer::input_size; ++j) {
        layer.weights_[i][j] -= alpha_ * gradient_layer.weights_[i][j];
      }
    }
    for (auto i = 0U; i < Layer::layer_size; ++i) {
      layer.bias_weight_[i] -= alpha_ * gradient_layer.bias_weight_[i];
    }
  }

  static constexpr auto const alpha_ = 50;
  static constexpr auto const number_of_layers_ = std::tuple_size<Layers>();
};

template <typename Layers>
struct ada_grad {
  void update(Layers const& gradient, Layers& nn) {
    constexpr auto const indices =
        std::make_index_sequence<number_of_layers_>();
    update_grad_squared(gradient, indices);
    update_weights(nn, gradient, indices);
  }

  template <std::size_t... I>
  void update_grad_squared(Layers const& gradient, std::index_sequence<I...>) {
    auto const update_grad_squared_layer = [](auto const& gradient_layer,
                                              auto& grad_squared_layer) {
      using Layer = std::decay_t<decltype(gradient_layer)>;
      for (auto i = 0U; i < Layer::layer_size; ++i) {
        for (auto j = 0U; j < Layer::input_size; ++j) {
          grad_squared_layer.weights_[i][j] +=
              gradient_layer.weights_[i][j] * gradient_layer.weights_[i][j];
        }
      }
      for (auto i = 0U; i < Layer::layer_size; ++i) {
        grad_squared_layer.bias_weight_[i] +=
            gradient_layer.bias_weight_[i] * gradient_layer.bias_weight_[i];
      }
    };
    (update_grad_squared_layer(std::get<I>(gradient),
                               std::get<I>(grad_squared_)),
     ...);
  }

  template <std::size_t... I>
  void update_weights(Layers& nn, Layers const& gradient,
                      std::index_sequence<I...>) {
    (update_weights_layer(std::get<I>(nn), std::get<I>(gradient),
                          std::get<I>(grad_squared_)),
     ...);
  }

  template <typename Layer>
  void update_weights_layer(Layer& layer, Layer const& gradient_layer,
                            Layer const& grad_squared_layer) {
    for (auto i = 0U; i < Layer::layer_size; ++i) {
      for (auto j = 0U; j < Layer::input_size; ++j) {
        layer.weights_[i][j] -=
            alpha_ * gradient_layer.weights_[i][j] /
            (std::sqrt(grad_squared_layer.weights_[i][j] + 1e-7));
      }
    }
    for (auto i = 0U; i < Layer::layer_size; ++i) {
      layer.bias_weight_[i] -=
          alpha_ * gradient_layer.bias_weight_[i] /
          (std::sqrt(grad_squared_layer.bias_weight_[i] + 1e-7));
    }
  }

  Layers grad_squared_{};
  static constexpr auto const alpha_ = 0.1;
  static constexpr auto const number_of_layers_ = std::tuple_size<Layers>();
};

template <typename Layers>
struct rms_prop {
  void update(Layers const& gradient, Layers& nn) {
    constexpr auto const indices =
        std::make_index_sequence<number_of_layers_>();
    update_grad_squared(gradient, indices);
    update_weights(nn, gradient, indices);
  }

  template <std::size_t... I>
  void update_grad_squared(Layers const& gradient, std::index_sequence<I...>) {
    auto const update_grad_squared_layer = [](auto const& gradient_layer,
                                              auto& grad_squared_layer) {
      using Layer = std::decay_t<decltype(gradient_layer)>;
      for (auto i = 0U; i < Layer::layer_size; ++i) {
        for (auto j = 0U; j < Layer::input_size; ++j) {
          grad_squared_layer.weights_[i][j] =
              alpha_ * grad_squared_layer.weights_[i][j] +
              (1 - alpha_) * gradient_layer.weights_[i][j] *
                  gradient_layer.weights_[i][j];
        }
      }
      for (auto i = 0U; i < Layer::layer_size; ++i) {
        grad_squared_layer.bias_weight_[i] +=
            alpha_ * grad_squared_layer.bias_weight_[i] +
            (1 - alpha_) * gradient_layer.bias_weight_[i] *
                gradient_layer.bias_weight_[i];
      }
    };
    (update_grad_squared_layer(std::get<I>(gradient),
                               std::get<I>(grad_squared_)),
     ...);
  }

  template <std::size_t... I>
  void update_weights(Layers& nn, Layers const& gradient,
                      std::index_sequence<I...>) {
    (update_weights_layer(std::get<I>(nn), std::get<I>(gradient),
                          std::get<I>(grad_squared_)),
     ...);
  }

  template <typename Layer>
  void update_weights_layer(Layer& layer, Layer const& gradient_layer,
                            Layer const& grad_squared_layer) {
    for (auto i = 0U; i < Layer::layer_size; ++i) {
      for (auto j = 0U; j < Layer::input_size; ++j) {
        layer.weights_[i][j] -=
            alpha_ * gradient_layer.weights_[i][j] /
            (std::sqrt(grad_squared_layer.weights_[i][j] + 1e-7));
      }
    }
    for (auto i = 0U; i < Layer::layer_size; ++i) {
      layer.bias_weight_[i] -=
          alpha_ * gradient_layer.bias_weight_[i] /
          (std::sqrt(grad_squared_layer.bias_weight_[i] + 1e-7));
    }
  }

  Layers grad_squared_{};
  static constexpr auto const alpha_ = 0.01;
  static constexpr auto const number_of_layers_ = std::tuple_size<Layers>();
};

}  // namespace chessbot
