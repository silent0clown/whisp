// zlib_util.h
#pragma once

#include <optional>
#include <string>

namespace whisker::util {

class ZlibUtil {
public:
    static std::optional<std::string> Compress(const std::string_view input);
    static std::optional<std::string> Decompress(const std::string_view input);
};

} // namespace whisker::util