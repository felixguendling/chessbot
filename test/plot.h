#pragma once

#include "utl/to_vec.h"

#if defined(CHESSBOT_PLOT)
#include <string_view>
#include <vector>

#include "matplot/matplot.h"

namespace chessbot {

struct plot {
  struct entry {
    unsigned iterations_;
    double error_;
  };

  explicit plot(std::string_view name) : name_{std::move(name)} {}

  void add_entry(unsigned const iterations, float const error) {
    entries_.emplace_back(entry{iterations, error});
  }

  void do_plot() {
    using namespace matplot;

    auto x = linspace(0, entries_.back().iterations_, entries_.size());
    auto y = utl::to_vec(entries_, [](entry const& e) { return e.error_; });

    scatter(x, y);

    show();

    std::ofstream out{std::string{name_} + ".csv"};
    for (auto const& [iterations, error] : entries_) {
      out << iterations << ";" << error << "\n";
    }
  }

  std::string_view name_;
  std::vector<entry> entries_;
};

}  // namespace chessbot

#else

#include <string_view>

namespace chessbot {

struct plot {
  explicit plot(std::string_view) {}
  void add_entry(unsigned const, float const) {}
  void do_plot() {}
};

}  // namespace chessbot

#endif