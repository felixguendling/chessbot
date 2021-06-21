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

    utl::verify(!entries_.empty() && std::all_of(begin(entries_), end(entries_),
                                                 [&](auto&& e) {
                                                   return e.size() ==
                                                          entries_[0].size();
                                                 }),
                "entries empty or not equal size");

    auto f = figure(true);
    //    legend(legend_);
    hold(on);
    for (auto const& [i, e] : utl::enumerate(entries_)) {
      auto x = linspace(0, entries_[0].back().iterations_, entries_[0].size());
      auto l1 =
          scatter(x, utl::to_vec(e, [](auto&& x) { return x.error_; }), 2);
      l1->display_name(legend_[i]);

      f->draw();
    }
    legend();
    show();
  }

  template <typename... Str>
  void add_legend(Str&&... str) {
    (legend_.template emplace_back(str), ...);
  }

  std::string_view name_;
  std::vector<std::vector<entry>> entries_;
  std::vector<std::string> legend_;
};

}  // namespace chessbot

#else

#include <string_view>

namespace chessbot {

struct plot {
  explicit plot(std::string_view) {}
  void add_entry(unsigned const, float const, unsigned i = 0) {}
  void do_plot() {}
  template <typename... Str>
  void add_legend(Str...) {}
};

}  // namespace chessbot

#endif
