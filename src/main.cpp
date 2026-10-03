#include <array>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <iostream>
#include <thread>

#include "streamlens/config.h"
#include "streamlens/network/udp_receiver.h"
#include "streamlens/version.h"

namespace {
std::atomic_bool running{true};

void handle_signal(int) {
    running = false;
}
}  // namespace

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

    std::signal(SIGINT, handle_signal);
#ifdef SIGTERM
    std::signal(SIGTERM, handle_signal);
#endif

    streamlens::UdpReceiver receiver;
    if (!receiver.open(config.input_port, error_message)) {
        std::cerr << "Error: " << error_message << '\n';
        return 1;
    }

    std::cout << std::unitbuf;
    std::cout << "StreamLens " << STREAMLENS_VERSION_STRING << '\n'
              << "UDP receiver listening on port " << config.input_port << '\n'
              << "Press Ctrl+C to stop.\n";

    std::array<std::byte, 65536> buffer{};
    std::uint64_t packet_count = 0;
    std::uint64_t total_bytes = 0;
    auto next_stats = std::chrono::steady_clock::now() +
                      std::chrono::seconds(config.stats_interval_seconds);

    while (running) {
        const int received = receiver.receive(buffer.data(), buffer.size(),
                                               error_message);
        if (received < 0) {
            std::cerr << "Error: " << error_message << '\n';
            return 1;
        }
        if (received > 0) {
            ++packet_count;
            total_bytes += static_cast<std::uint64_t>(received);
        }

        const auto now = std::chrono::steady_clock::now();
        if (now >= next_stats) {
            std::cout << "[STAT] packets=" << packet_count
                      << " bytes=" << total_bytes << '\n';
            next_stats = now +
                         std::chrono::seconds(config.stats_interval_seconds);
        }
    }

    receiver.close();
    std::cout << "Stopped. packets=" << packet_count
              << " bytes=" << total_bytes << '\n';
    return 0;
}
