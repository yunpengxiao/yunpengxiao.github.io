#include "path_helper.h"

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#ifdef PathAppend
#undef PathAppend
#endif

std::string PathHelper::PathAppend (const std::string &base,
    const std::string &more)
{
    char path[MAX_PATH];
    strncpy (path, base.c_str (), MAX_PATH);

    PathAppendA (path, more.c_str ());
    return path;
}
