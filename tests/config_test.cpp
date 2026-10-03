#include <cassert>
#include <string>

#include "streamlens/config.h"

int main() {
    streamlens::AppConfig config;
    std::string error;

    const char program[] = "streamlens";
    const char input_option[] = "--input-port";
    const char input_value[] = "6000";
    const char host_option[] = "--output-host";
    const char host_value[] = "192.168.1.10";
    const char output_option[] = "--output-port";
    const char output_value[] = "7000";
    const char stats_option[] = "--stats-interval";
    const char stats_value[] = "5";

    char* argv[] = {
        const_cast<char*>(program),
        const_cast<char*>(input_option),
        const_cast<char*>(input_value),
        const_cast<char*>(host_option),
        const_cast<char*>(host_value),
        const_cast<char*>(output_option),
        const_cast<char*>(output_value),
        const_cast<char*>(stats_option),
        const_cast<char*>(stats_value),
    };

    const auto status = streamlens::parse_command_line(
        static_cast<int>(sizeof(argv) / sizeof(argv[0])), argv, config, error);

    assert(status == streamlens::ParseStatus::Success);
    assert(config.input_port == 6000);
    assert(config.output_host == "192.168.1.10");
    assert(config.output_port == 7000);
    assert(config.stats_interval_seconds == 5);

    return 0;
}
