#include <iostream>

#include "streamlens/config.h"
#include "streamlens/version.h"

int main(int argc, char* argv[]) {
    streamlens::AppConfig config;
    std::string error_message;
    const auto status = streamlens::parse_command_line(
        argc, argv, config, error_message);

    if (status == streamlens::ParseStatus::HelpRequested) {
        std::cout << streamlens::usage(argv[0]);
        return 0;
    }

    if (status == streamlens::ParseStatus::VersionRequested) {
        std::cout << "StreamLens " << STREAMLENS_VERSION_STRING << '\n';
        return 0;
    }

    if (status == streamlens::ParseStatus::Error) {
        std::cerr << "Error: " << error_message << "\n\n"
                  << streamlens::usage(argv[0]);
        return 2;
    }

    std::cout << "StreamLens " << STREAMLENS_VERSION_STRING << '\n'
              << "Video stream relay and diagnostics toolkit\n"
              << "Input UDP port: " << config.input_port << '\n'
              << "Output: " << config.output_host << ':' << config.output_port
              << '\n'
              << "Statistics interval: " << config.stats_interval_seconds
              << " second(s)\n";
    return 0;
}
