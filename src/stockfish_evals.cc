#include "chessbot/stockfish_evals.h"

#include <charconv>

#include "boost/asio.hpp"
#include "boost/process.hpp"

#include "utl/verify.h"

#include "chessbot/position.h"

namespace bp = boost::process;

namespace chessbot {

std::map<std::string, float> stockfish_evals(position const& p) {
  boost::asio::io_service ios;

  bp::async_pipe input(ios);
  bp::async_pipe ap(ios);
  boost::asio::streambuf buf;

  bp::child c(bp::search_path("stockfish"), bp::std_in<input, bp::std_out> ap);

  auto const write = [&](std::string const& str) {
    boost::asio::async_write(
        input, boost::asio::buffer(str),
        [&](boost::system::error_code const& ec, std::size_t size) {});
  };
  auto lines_result = std::map<std::string, float>{};
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

          if (line.find("bestmove") != std::string::npos) {
            write("quit\n");
            return;
          }
        });
  };

  auto const parse_line = [&](std::string const& line) {
    constexpr auto const depth_str = std::string_view{"depth"};
    auto const depth_pos = line.find(depth_str);
    if (depth_pos == std::string::npos) {
      return;
    }
    auto const depth_out = line.substr(depth_pos + depth_str.length() + 1);
    auto depth = 0U;
    auto const [rest, ec_r] =
        std::from_chars(&depth_out.front(), &depth_out.back(), depth);
    if (ec_r != std::errc()) {
      return;
    }

    if (depth != 18) {
      return;
    }

    auto const wdl_pos = line.find("wdl");
    if (wdl_pos == std::string::npos) {
      return;
    }

    auto w = 0U;
    auto d = 0U;
    auto l = 0U;

    auto const [w_rest, w_ec] =
        std::from_chars(&line[wdl_pos + 4], &line.back(), w);
    if (w_ec != std::errc()) {
      return;
    }

    auto const [d_rest, d_ec] = std::from_chars(w_rest + 1, &line.back(), d);
    if (d_ec != std::errc()) {
      return;
    }

    auto const [l_rest, l_ec] = std::from_chars(d_rest + 1, &line.back(), l);
    if (l_ec != std::errc()) {
      return;
    }

    auto const pv_pos = line.find(" pv");
    if (pv_pos == std::string::npos) {
      return;
    }

    lines_result.emplace(line.substr(pv_pos + 4, 4), (w + 0.5 * d) / 1000);
  };

  read();
  ios.run();
  c.wait();
  int result = c.exit_code();
  utl::verify(result == 0, "stockfish exit code {}", result);

  std::string line;
  while (std::getline(output, line)) {
    parse_line(line);
  }

  return lines_result;
}

}  // namespace chessbot