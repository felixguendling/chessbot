#include "chessbot/stockfish_evals.h"

#include <charconv>

#include "boost/asio.hpp"
#include "boost/process.hpp"

#include "utl/verify.h"

#include "chessbot/position.h"

namespace bp = boost::process;

namespace chessbot {

std::map<std::string, move_eval> stockfish_evals(position const& p) {
  boost::asio::io_service ios;

  bp::async_pipe input(ios);
  bp::async_pipe ap(ios);
  boost::asio::streambuf buf;

  bp::child c(bp::search_path("stockfish"), bp::std_in<input, bp::std_out> ap);

  auto t = boost::asio::deadline_timer{ios, boost::posix_time::seconds{60}};

  auto in = std::stringstream{};
  auto const write = [&](std::string const& str) {
    in << str;
    boost::asio::async_write(
        input, boost::asio::buffer(str),
        [&](boost::system::error_code const& ec, std::size_t size) {});
  };
  auto lines_result = std::map<std::string, move_eval>{};
  write("setoption name UCI_ShowWDL value true\n");
  write("setoption name MultiPv value 300\n");
  write("position fen " + p.to_fen() + "\n");
  write("go depth 18\n");

  auto output = std::stringstream{};

  std::function<void()> read = [&]() {
    boost::asio::async_read_until(
        ap, buf, '\n',
        [&](const boost::system::error_code& ec, std::size_t size) {
          if (ec) {
            return;
          }
          read();
          auto const line = std::string{
              boost::asio::buffer_cast<const char*>(buf.data()), buf.size()};
          buf.consume(buf.size());

          output << line;

          if (output.str().find("bestmove") != std::string::npos) {
            t.cancel();
            write("quit\n");
            return;
          }
        });
  };

  auto killed = false;
  t.async_wait([&](boost::system::error_code const& ec) {
    if (ec != boost::asio::error::operation_aborted) {
      c.terminate();
      killed = true;
    }
  });

  read();
  ios.run();
  c.wait();
  int result = c.exit_code();

  if (killed || result != 0) {
    std::cerr << "STOCKFISH PROBLEM:\n";
    std::cerr << "IN:\n" << in.str() << "\n";
    std::cerr << "OUT:\n" << output.str() << "\n";
  }

  utl::verify(!killed, "killed stockfish due to timeout");
  utl::verify(result == 0, "stockfish exit code {}", result);

  std::string line;
  while (std::getline(output, line)) {
    constexpr auto const depth_str = std::string_view{"depth"};
    auto const depth_pos = line.find(depth_str);
    if (depth_pos == std::string::npos) {
      continue;
    }
    auto const depth_out = line.substr(depth_pos + depth_str.length() + 1);
    auto depth = 0U;
    auto const [rest, ec_r] =
        std::from_chars(&depth_out.front(), &depth_out.back(), depth);
    if (ec_r != std::errc()) {
      continue;
    }

    if (depth != 18) {
      continue;
    }

    auto const cp_pos = line.find("cp ");
    auto const mate_pos = line.find("mate ");
    if (cp_pos == std::string::npos && mate_pos == std::string::npos) {
      continue;
    }

    auto cp = 0;
    if (cp_pos != std::string::npos) {
      auto const [rest, ec] =
          std::from_chars(&line[cp_pos + 3], &line.back(), cp);

      if (ec != std::errc()) {
        continue;
      }
    }

    auto mate = 0;
    if (mate_pos != std::string::npos) {
      auto const [rest, ec] =
          std::from_chars(&line[mate_pos + 5], &line.back(), mate);
      if (ec != std::errc()) {
        continue;
      }
    }

    auto const wdl_pos = line.find("wdl");
    if (wdl_pos == std::string::npos) {
      continue;
    }

    auto w = 0U;
    auto d = 0U;
    auto l = 0U;

    auto const [w_rest, w_ec] =
        std::from_chars(&line[wdl_pos + 4], &line.back(), w);
    if (w_ec != std::errc()) {
      continue;
    }

    auto const [d_rest, d_ec] = std::from_chars(w_rest + 1, &line.back(), d);
    if (d_ec != std::errc()) {
      continue;
    }

    auto const [l_rest, l_ec] = std::from_chars(d_rest + 1, &line.back(), l);
    if (l_ec != std::errc()) {
      continue;
    }

    auto const pv_pos = line.find(" pv");
    if (pv_pos == std::string::npos) {
      continue;
    }

    lines_result.emplace(line.substr(pv_pos + 4, 4),
                         move_eval{.mate_ = mate, .cp_ = cp});
  }

  return lines_result;
}

}  // namespace chessbot
