// Functional Transpose by BOT Man, 2018
// clang++ functional-iostream.cpp -std=c++11 -Wall -o iostream && ./iostream

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

int main(int argc, char* argv[]) {
  std::ifstream ifs = argc >= 2 ? std::ifstream(argv[1]) : std::ifstream();
  std::ofstream ofs = argc >= 3 ? std::ofstream(argv[2]) : std::ofstream();
  std::copy(std::istream_iterator<std::string>(ifs),
            std::istream_iterator<std::string>(),
            std::ostream_iterator<std::string>(ofs, "\n"));

#ifdef NOT_COMPILE
  std::copy(std::istream_iterator<std::string>(
                argc >= 2 ? std::ifstream(argv[1]) : std::ifstream()),
            std::istream_iterator<std::string>(),
            std::ostream_iterator<std::string>(
                argc >= 3 ? std::ofstream(argv[2]) : std::ofstream(), "\n"));
#endif  // NOT_COMPILE

  auto io_wrapper = [](std::ifstream&& ifs, std::ofstream&& ofs) -> void {
    std::copy(std::istream_iterator<std::string>(ifs),
              std::istream_iterator<std::string>(),
              std::ostream_iterator<std::string>(ofs, "\n"));
  };
  io_wrapper(argc >= 2 ? std::ifstream(argv[1]) : std::ifstream(),
             argc >= 3 ? std::ofstream(argv[2]) : std::ofstream());

  return 0;
}
