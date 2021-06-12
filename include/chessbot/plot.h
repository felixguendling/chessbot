#pragma once

#include "utl/enumerate.h"
#include "utl/to_vec.h"
#include "utl/verify.h"

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

  void add_entry(unsigned const iterations, float const error,
                 unsigned dataset = 0) {
    if (entries_.size() <= dataset) {
      entries_.resize(dataset + 1);
    }
    entries_[dataset].emplace_back(entry{iterations, error});
  }

  void do_plot() {
    using namespace matplot;

    for (auto const& e : entries_) {
      std::cout << "size: " << e.size() << "\n";
    }
    utl::verify(!entries_.empty() && std::all_of(begin(entries_), end(entries_),
                                                 [&](auto&& e) {
                                                   return e.size() ==
                                                          entries_[0].size();
                                                 }),
                "entries empty or not equal size");

    auto all_entries = std::vector<double>{};
    for (auto const& e : entries_) {
      for (auto const& x : e) {
        all_entries.emplace_back(x.error_);
      }
    }

    auto x = linspace(0, entries_[0].back().iterations_, entries_[0].size());
    auto all_x = std::vector<double>{};
    for (auto const& e : entries_) {
      all_x.insert(end(all_x), begin(x), end(x));
    }

    auto all_colors = std::vector<double>{};
    auto all_c_idx = 0U;
    all_colors.resize(entries_[0].size() * entries_.size());
    for (auto const& [i, e] : utl::enumerate(entries_)) {
      for (auto j = 0; j != e.size(); ++j) {
        all_colors[all_c_idx++] = i;
      }
    }

    scatter(all_x, all_entries, 6, all_colors);

    legend();

    show();

    //    std::ofstream out{std::string{name_} + ".csv"};
    //    for (auto const& [iterations, error] : entries_) {
    //      out << iterations << ";" << error << "\n";
    //    }
  }

  std::string_view name_;
  std::vector<std::vector<entry>> entries_;
};

}  // namespace chessbot

#else

#include <string_view>

namespace chessbot {

struct plot {
  explicit plot(std::string_view) {}
  void add_entry(unsigned const, float const, unsigned i = 0) {}
  void do_plot() {}
};

}  // namespace chessbot

#endif
