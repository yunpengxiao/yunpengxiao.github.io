
#include <algorithm>
#include "my_module.h"
#include "path_helper.h"

#ifdef min
#undef min
#endif

int main (int argc, char *argv[]) {
    auto validArgumentCount = std::min (argc, 2);
    auto workingDirectory = PathHelper::PathAppend (argv[0], "..\\");
    return 0;
}
