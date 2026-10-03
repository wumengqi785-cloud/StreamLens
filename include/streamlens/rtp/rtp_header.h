#pragma once

#include <cstddef>
#include <cstdint>

namespace streamlens {

struct RtpHeader {
    std::uint8_t version{0};
    bool padding{false};
    bool extension{false};
    std::uint8_t csrc_count{0};
    bool marker{false};
    std::uint8_t payload_type{0};
    std::uint16_t sequence_number{0};
    std::uint32_t timestamp{0};
    std::uint32_t ssrc{0};
    std::size_t header_size{0};
    std::size_t payload_offset{0};
    std::size_t payload_size{0};
    std::size_t padding_size{0};
};

}  // namespace streamlens
