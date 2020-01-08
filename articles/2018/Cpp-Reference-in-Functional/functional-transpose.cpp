// Functional Transpose by BOT Man, 2018
// clang++ functional-transpose.cpp -std=c++11 -Wall -o transpose && ./transpose

#include <algorithm>
#include <iostream>
#include <map>
#include <numeric>

class CopyGuard {
 public:
  explicit CopyGuard(int flag) : flag_(flag) {
    std::cout << "ctor" << flag_ << std::endl;
  }
  CopyGuard(const CopyGuard& rhs) {
    flag_ = rhs.flag_;
    std::cout << "copy" << flag_ << std::endl;
  }
  CopyGuard(CopyGuard&& rhs) {
    flag_ = rhs.flag_;
    std::cout << "move" << flag_ << std::endl;
  }
#ifdef SHOW_DTOR
  ~CopyGuard() { std::cout << "dtor" << flag_ << std::endl; }
#endif  // SHOW_DTOR

  bool operator==(const CopyGuard& rhs) const { return flag_ == rhs.flag_; }
  bool operator<(const CopyGuard& rhs) const { return flag_ < rhs.flag_; }

 private:
  int flag_;
};

namespace cpp20 {
// https://en.cppreference.com/w/cpp/algorithm/accumulate
template <class InputIt, class T, class BinaryOperation>
T accumulate(InputIt first, InputIt last, T init, BinaryOperation op) {
  for (; first != last; ++first)
    init = op(std::move(init), *first);  // std::move since C++20
  return init;
}
}  // namespace move

template <typename K>
using Matrix = std::multimap<K, K>;

template <typename K>
using Edge = typename Matrix<K>::value_type;

template <typename K>
Matrix<K> TransposeImperative(const Matrix<K>& matrix) {
  Matrix<K> transposed{};
  for (const Edge<K>& edge : matrix) {
    transposed.emplace(edge.second, edge.first);
  }
  return transposed;
}

template <typename K>
Matrix<K> TransposeFunctional(const Matrix<K>& matrix) {
  return std::accumulate(
      std::begin(matrix), std::end(matrix), Matrix<K>{},
      [](const Matrix<K>& transposed, const Edge<K>& edge) -> Matrix<K> {
        Matrix<K> new_transposed = transposed;
        new_transposed.emplace(edge.second, edge.first);
        return new_transposed;
      });
}

template <typename K>
Matrix<K> TransposeLref(const Matrix<K>& matrix) {
  return std::accumulate(
      std::begin(matrix), std::end(matrix), Matrix<K>{},
      [](Matrix<K>& transposed, const Edge<K>& edge) -> Matrix<K>& {
        transposed.emplace(edge.second, edge.first);
        return transposed;
      });
}

template <typename K>
Matrix<K> TransposeRref(const Matrix<K>& matrix) {
  return cpp20::accumulate(
      std::begin(matrix), std::end(matrix), Matrix<K>{},
      [](Matrix<K>&& transposed, const Edge<K>& edge) -> Matrix<K> {
        transposed.emplace(edge.second, edge.first);
        return std::move(transposed);
      });
}

int main(int argc, char* argv[]) {
  std::cout << "checkpoint: Matrix\n";
  auto matrix = Matrix<CopyGuard>{
      {CopyGuard{11}, CopyGuard{12}},
      {CopyGuard{21}, CopyGuard{22}},
      {CopyGuard{31}, CopyGuard{32}},
  };

  std::cout << "checkpoint: CopyMatrix\n";
  auto copied = matrix;

  std::cout << "checkpoint: TransposeImperative\n";
  auto imperative = TransposeImperative(matrix);

  std::cout << "checkpoint: TransposeFunctional\n";
  auto functional = TransposeFunctional(matrix);

  std::cout << "checkpoint: TransposeLref\n";
  auto lvalue_ref = TransposeLref(matrix);

  std::cout << "checkpoint: TransposeRref\n";
  auto rvalue_ref = TransposeRref(matrix);

  std::cout << "checkpoint: return\n";
  return 0;
}
