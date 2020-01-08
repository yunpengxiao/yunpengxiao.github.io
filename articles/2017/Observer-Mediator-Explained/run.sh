find -name '*.cpp' | while read name; do clang++ $name -std=c++14 -o test && ./test && rm test; done
