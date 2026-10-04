#pragma once

#include <cstddef>
#include <cstdint>

#include "streamlens/rtp/rtp_header.h"

namespace streamlens {

class StreamStatistics {
public:
    void reset() noexcept;

    void update(const RtpHeader& header, std::size_t packet_size) noexcept;

    std::uint64_t packet_count() const noexcept;
    std::uint64_t byte_count() const noexcept;
    std::uint64_t lost_packet_count() const noexcept;
    double loss_rate() const noexcept;

    bool has_sequence() const noexcept;
    std::uint16_t highest_sequence_number() const noexcept;
    std::uint8_t payload_type() const noexcept;
    std::uint32_t ssrc() const noexcept;

private:
    std::uint64_t packet_count_{0};
    std::uint64_t byte_count_{0};
    std::uint64_t lost_packet_count_{0};
    bool has_sequence_{false};
    std::uint16_t highest_sequence_number_{0};
    std::uint8_t payload_type_{0};
    std::uint32_t ssrc_{0};
};

}  // namespace streamlens
