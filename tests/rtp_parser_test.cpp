#include <cassert>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "streamlens/rtp/rtp_parser.h"

namespace {

std::byte byte(std::uint8_t value) {
    return static_cast<std::byte>(value);
}

void test_basic_header() {
    const std::vector<std::byte> packet{
        byte(0x80), byte(0xE0), byte(0x01), byte(0x02),
        byte(0x00), byte(0x01), byte(0x5F), byte(0x90),
        byte(0x12), byte(0x34), byte(0x56), byte(0x78),
        byte(0xAA), byte(0xBB),
    };

    streamlens::RtpHeader header;
    std::string error;
    assert(streamlens::parse_rtp_header(
        packet.data(), packet.size(), header, error));
    assert(header.version == 2);
    assert(!header.padding);
    assert(!header.extension);
    assert(header.marker);
    assert(header.payload_type == 96);
    assert(header.sequence_number == 0x0102);
    assert(header.timestamp == 0x00015F90);
    assert(header.ssrc == 0x12345678);
    assert(header.header_size == 12);
    assert(header.payload_offset == 12);
    assert(header.payload_size == 2);
}

void test_csrc_and_extension() {
    const std::vector<std::byte> packet{
        byte(0x92), byte(0x60), byte(0x00), byte(0x01),
        byte(0x00), byte(0x00), byte(0x00), byte(0x01),
        byte(0x00), byte(0x00), byte(0x00), byte(0x02),
        byte(0x00), byte(0x00), byte(0x00), byte(0x03),
        byte(0x00), byte(0x00), byte(0x00), byte(0x04),
        byte(0xBE), byte(0xDE), byte(0x00), byte(0x01),
        byte(0x10), byte(0x20), byte(0x30), byte(0x40),
        byte(0x99),
    };

    streamlens::RtpHeader header;
    std::string error;
    assert(streamlens::parse_rtp_header(
        packet.data(), packet.size(), header, error));
    assert(header.csrc_count == 2);
    assert(header.extension);
    assert(header.header_size == 28);
    assert(header.payload_size == 1);
}

void test_padding() {
    const std::vector<std::byte> packet{
        byte(0xA0), byte(0x60), byte(0x00), byte(0x01),
        byte(0x00), byte(0x00), byte(0x00), byte(0x01),
        byte(0x00), byte(0x00), byte(0x00), byte(0x02),
        byte(0x11), byte(0x22), byte(0x00), byte(0x00), byte(0x02),
    };

    streamlens::RtpHeader header;
    std::string error;
    assert(streamlens::parse_rtp_header(
        packet.data(), packet.size(), header, error));
    assert(header.padding);
    assert(header.padding_size == 2);
    assert(header.payload_size == 3);
}

void test_invalid_packets() {
    streamlens::RtpHeader header;
    std::string error;
    const std::vector<std::byte> short_packet(11);
    assert(!streamlens::parse_rtp_header(
        short_packet.data(), short_packet.size(), header, error));

    const std::vector<std::byte> wrong_version(12, byte(0));
    assert(!streamlens::parse_rtp_header(
        wrong_version.data(), wrong_version.size(), header, error));
}

}  // namespace

int main() {
    test_basic_header();
    test_csrc_and_extension();
    test_padding();
    test_invalid_packets();
    return 0;
}
