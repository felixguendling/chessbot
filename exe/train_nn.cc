#include <cista/mmap.h>
#include <iostream>
#include <memory>

#include "utl/enumerate.h"

#include "chessbot/nn.h"
#include "chessbot/nn_chess.h"
#include "chessbot/plot.h"
#include "chessbot/read_training_set.h"

using namespace chessbot;

constexpr auto const batch_size = size_t{100U};
constexpr auto const inner_loop_size = 100000;
constexpr auto const learning_rate = 0.53;

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cout << "usage: " << argv[0] << " [TRAINING_FILE]\n";
    return 1;
  }

  std::cout << "reading training set " << argv[1] << " ...\n";
  std::ifstream in{argv[1]};
  auto const training_set = read_training_set(in);

  auto counts = std::map<std::string, unsigned>{};
  for (auto const& [p, evals] : training_set) {
    ++counts[p.to_fen()];
  }
  for (auto const& [fen, count] : counts) {
    if (count != 1) {
      std::cout << "duplicate fen: " << fen << "   ===> " << count << "\n";
    }
  }

  if (training_set.size() < batch_size) {
    std::cout << "insufficient training samples (#batch=" << batch_size
              << ", #samples=" << training_set.size() << ")\n";
    return 1;
  }

  std::cout << "building training data ...\n";
  auto input = std::array<std::array<real_t, input_size>, batch_size>{};
  auto expected = std::array<std::array<real_t, output_size>, batch_size>{};

  for (auto i = 0; i < batch_size; ++i) {
    auto const& [p, moves] = training_set[i];
    input[i] = nn_input_from_position(p);
    expected[i] = to_expected(moves);
  }

  auto pl_absolute_errors = plot{""};
  auto pl_max_errors = plot{""};

  auto n = std::make_unique<network<input_size, 16, 16, output_size>>();

  std::cout << "training network ...\n";
  n->train_epoch(
      input, expected, learning_rate, inner_loop_size, [&](unsigned i) {
        if (i != 0 && i % std::max(1, inner_loop_size / 100) == 0) {
          const auto [e1, e2] = determine_error(*n, input, expected);
          pl_absolute_errors.add_entry(i, e1);
          pl_max_errors.add_entry(i, e2);
          std::cout << "\r" << i << " / " << inner_loop_size << std::flush;
        }
      });

  std::cout << "\n";

  std::cout << "writing trained weights ...\n";
  {
    auto out = cista::mmap{"weights.bin"};
    out.resize(sizeof(*n));
    std::memcpy(out.data(), &*n, sizeof(*n));
  }

  std::cout << "building statistics ...\n";
  auto test_fens = std::array<std::string, batch_size>{};
  for (auto const& [i, entry] : utl::enumerate(training_set)) {
    if (i == test_fens.size()) {
      break;
    }
    auto const& [p, moves] = entry;
    test_fens[i] = p.to_fen();
  }
  print_move_evals_sorted_by_error(printed_errors, *n, test_fens, expected);

  pl_absolute_errors.do_plot();
  pl_max_errors.do_plot();
}
