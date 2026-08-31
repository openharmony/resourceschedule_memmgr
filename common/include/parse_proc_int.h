#ifndef PARSE_PROC_INT_H
#define PARSE_PROC_INT_H

#include <charconv>
#include <cstdint>
#include <string>
#include <system_error>

inline bool ParseProcInt32(const char *text, int32_t &out)
{
    if (text == nullptr || text[0] == '\0') {
        return false;
    }
    int32_t value = 0;
    const char *last = text + std::char_traits<char>::length(text);
    auto result = std::from_chars(text, last, value);
    if (result.ec != std::errc() || result.ptr != last) {
        return false;
    }
    out = value;
    return true;
}

inline bool ParseProcInt32(const std::string &s, int32_t &out)
{
    return ParseProcInt32(s.c_str(), out);
}

#endif
