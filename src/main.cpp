#include <array>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <iomanip>
#include <iostream>

#include "streamlens/config.h"
#include "streamlens/network/udp_receiver.h"
#include "streamlens/network/udp_sender.h"
#include "streamlens/rtp/rtp_parser.h"
#include "streamlens/statistics/stream_statistics.h"
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

    streamlens::UdpSender sender;
    if (!sender.open(config.output_host, config.output_port, error_message)) {
        std::cerr << "Error: " << error_message << '\n';
        return 1;
    }

    std::cout << std::unitbuf;
    std::cout << "StreamLens " << STREAMLENS_VERSION_STRING << '\n'
              << "UDP receiver listening on port " << config.input_port << '\n'
              << "Press Ctrl+C to stop.\n";

    std::array<std::byte, 65536> buffer{};
    streamlens::StreamStatistics statistics;
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
            streamlens::RtpHeader rtp_header;
            if (streamlens::parse_rtp_header(
                    buffer.data(), static_cast<std::size_t>(received),
                    rtp_header, error_message)) {
                statistics.update(
                    rtp_header, static_cast<std::size_t>(received));
            } else {
                std::cerr << "[WARN] invalid RTP packet: "
                          << error_message << '\n';
            }

            if (sender.send(buffer.data(), static_cast<std::size_t>(received),
                            error_message) < 0) {
                std::cerr << "Error: " << error_message << '\n';
                return 1;
            }
        }

        const auto now = std::chrono::steady_clock::now();
        if (now >= next_stats) {
            std::cout << std::fixed << std::setprecision(2)
                      << "[STAT] packets=" << statistics.packet_count()
                      << " bytes=" << statistics.byte_count()
                      << " lost=" << statistics.lost_packet_count()
                      << " loss_rate=" << statistics.loss_rate() << "%";
            if (statistics.has_sequence()) {
                std::cout << " highest_seq="
                          << statistics.highest_sequence_number();
            }
            std::cout << " payload_type=" << static_cast<unsigned>(
                             statistics.payload_type())
                      << " ssrc=" << statistics.ssrc() << '\n';
            next_stats = now +
                         std::chrono::seconds(config.stats_interval_seconds);
        }
    }

    receiver.close();
    sender.close();
    std::cout << "Stopped. packets=" << statistics.packet_count()
              << " bytes=" << statistics.byte_count()
              << " lost=" << statistics.lost_packet_count() << '\n';
    return 0;
}
