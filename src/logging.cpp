/*
    logging.cpp is a part of spiral by techhog.

    Copyright (c) 2026 techhog
    https://github.com/TechHog8984/spiral
    See the GPLv3 LICENSE file.
*/

#include "logging.hpp"

#include <cstring>
#include <string>

namespace spiral::logging {

void print(const std::string &content) {
    fprintf(stdout, "[spiral] ");
    fwrite(content.c_str(), content.size(), 1, stdout);

    char *buffer = static_cast<char *>(malloc(content.size()));
    if (buffer)
        memcpy(buffer, content.data(), content.size());
    else
        return;

    log_list.push_back({.content = buffer, .len = content.size()});
    return;
}

void cleanup() {
    for (const auto &log : log_list)
        free(log.content);
}

}  // namespace spiral::logging
