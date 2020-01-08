#include <algorithm>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>

template <typename Container, typename Predicate>
bool every(const Container& source, Predicate predicate) {
  return std::all_of(std::begin(source), std::end(source), predicate);
}

template <typename Container, typename Predicate>
bool some(const Container& source, Predicate predicate) {
  return std::any_of(std::begin(source), std::end(source), predicate);
}

template <typename Container, typename Function>
Container map(const Container& source, Function function) {
  Container ret;
  std::transform(std::begin(source), std::end(source), std::back_inserter(ret),
                 function);
  return ret;
}

template <typename Container, typename Predicate>
Container filter(const Container& source, Predicate predicate) {
  Container ret;
  std::copy_if(std::begin(source), std::end(source), std::back_inserter(ret),
               predicate);
  return ret;
}

template <typename Container, typename Target, typename Function>
Target reduce(const Container& source, Target init, Function function) {
  return std::accumulate(std::begin(source), std::end(source), init, function);
}

template <typename Container, typename Function>
void for_each(const Container& source, Function function) {
  std::for_each(std::begin(source), std::end(source), function);
}

int main() {
  const std::vector<int> source{1, 2, 3, 4, 5, 6};

  std::cout << "every (positive):" << std::boolalpha
            << every(source, [](const auto& e) { return e > 0; }) << std::endl;

  std::cout << "every (odd):" << std::boolalpha
            << every(source, [](const auto& e) { return e % 2; }) << std::endl;

  std::cout << "some (odd):" << std::boolalpha
            << some(source, [](const auto& e) { return e % 2; }) << std::endl;

  std::cout << "some (negative):" << std::boolalpha
            << some(source, [](const auto& e) { return e < 0; }) << std::endl;

  std::cout << "map (square):";
  for_each(map(source, [](const auto& e) { return e * e; }),
           [](const auto& e) { std::cout << " " << e; });
  std::cout << std::endl;

  std::cout << "filter (odd):";
  for_each(filter(source, [](const auto& e) { return e % 2; }),
           [](const auto& e) { std::cout << " " << e; });
  std::cout << std::endl;

  std::cout << "reduce (sum):"
            << reduce(source, 0,
                      [](const auto& acc, const auto& e) { return acc + e; })
            << std::endl;

  return 0;
}
