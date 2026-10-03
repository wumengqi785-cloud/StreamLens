#include "streamlens/config.h"

#include <charconv>
#include <limits>
#include <string>

namespace streamlens {
namespace {

template <typename Integer>
bool parse_positive_integer(std::string_view value, Integer& result) {
    if (value.empty() || value.front() == '-') {
        return false;
    }

    Integer parsed{};
    const auto* begin = value.data();
    const auto* end = value.data() + value.size();
    const auto conversion = std::from_chars(begin, end, parsed);

    if (conversion.ec != std::errc{} || conversion.ptr != end || parsed == 0) {
        return false;
    }

    result = parsed;
    return true;
}

bool read_value(int argc,
                char* const argv[],
                int& index,
                std::string_view option,
                std::string_view& value,
                std::string& error_message) {
    if (index + 1 >= argc) {
        error_message = "missing value for option '" + std::string(option) + "'";
        return false;
    }

    value = argv[++index];
    if (value.empty() || value.front() == '-') {
        error_message = "invalid value for option '" + std::string(option) + "'";
        return false;
    }

    return true;
}

}  // namespace

ParseStatus parse_command_line(int argc,
                               char* const argv[],
                               AppConfig& config,
                               std::string& error_message) {
    for (int index = 1; index < argc; ++index) {
        const std::string_view option{argv[index]};

        if (option == "--help" || option == "-h") {
            return ParseStatus::HelpRequested;
        }

        if (option == "--version" || option == "-v") {
            return ParseStatus::VersionRequested;
        }

        std::string_view value;
        if (option == "--input-port") {
            if (!read_value(argc, argv, index, option, value, error_message) ||
                !parse_positive_integer(value, config.input_port)) {
                if (error_message.empty()) {
                    error_message = "input port must be an integer from 1 to 65535";
                }
                return ParseStatus::Error;
            }
            continue;
        }

        if (option == "--output-host") {
            if (!read_value(argc, argv, index, option, value, error_message)) {
                return ParseStatus::Error;
            }
            config.output_host = value;
            continue;
        }

        if (option == "--output-port") {
            if (!read_value(argc, argv, index, option, value, error_message) ||
                !parse_positive_integer(value, config.output_port)) {
                if (error_message.empty()) {
                    error_message = "output port must be an integer from 1 to 65535";
                }
                return ParseStatus::Error;
            }
            continue;
        }

        if (option == "--stats-interval") {
            if (!read_value(argc, argv, index, option, value, error_message) ||
                !parse_positive_integer(value, config.stats_interval_seconds)) {
                if (error_message.empty()) {
                    error_message = "stats interval must be a positive integer";
                }
                return ParseStatus::Error;
            }
            continue;
        }

        error_message = "unknown option '" + std::string(option) + "'";
        return ParseStatus::Error;
    }

    return ParseStatus::Success;
}

std::string usage(std::string_view program_name) {
    return "Usage: " + std::string(program_name) +
           " [options]\n"
           "\n"
           "Options:\n"
           "  --input-port PORT       UDP input port (default: 5000)\n"
           "  --output-host HOST      UDP output host (default: 127.0.0.1)\n"
           "  --output-port PORT      UDP output port (default: 9000)\n"
           "  --stats-interval SEC    Statistics interval (default: 1)\n"
           "  -h, --help              Show this help message\n"
           "  -v, --version           Show version information\n";
}

}  // namespace streamlens
