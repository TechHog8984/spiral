/*
    logging.hpp is a part of spiral by techhog.

    Copyright (c) 2026 techhog
    https://github.com/TechHog8984/spiral
    See the GPLv3 LICENSE file.
*/

#pragma once

#include <string>
#include <vector>

namespace spiral::logging {

struct Log {
    char *content;
    size_t len;
};

inline std::vector<Log> log_list;

void print(const std::string &content);

void cleanup();

}  // namespace spiral::logging
