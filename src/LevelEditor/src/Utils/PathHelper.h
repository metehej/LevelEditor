#ifndef PATH_HELPER_H
#define PATH_HELPER_H

#include <string>

inline static std::string GetAbsolutePath(const std::string& relativePath) {
    return ROOT_PATH + relativePath;
}

#endif