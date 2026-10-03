#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace streamlens {

struct AppConfig {
    std::uint16_t input_port{5000};
    std::string output_host{"127.0.0.1"};
    std::uint16_t output_port{9000};
    std::uint32_t stats_interval_seconds{1};
};

enum class ParseStatus {
    Success,
    HelpRequested,
    VersionRequested,
    Error,
};

ParseStatus parse_command_line(int argc,
                               char* const argv[],
                               AppConfig& config,
                               std::string& error_message);

std::string usage(std::string_view program_name);

}  // namespace streamlens
