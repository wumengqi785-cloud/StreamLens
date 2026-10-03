#include "streamlens/rtp/rtp_parser.h"

#include <cstdint>

namespace streamlens {
namespace {

constexpr std::size_t MinimumRtpHeaderSize = 12;

std::uint8_t byte_at(const std::byte* data, std::size_t index) {
    return std::to_integer<std::uint8_t>(data[index]);
}

std::uint16_t read_u16_be(const std::byte* data, std::size_t offset) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(byte_at(data, offset)) << 8) |
        byte_at(data, offset + 1));
}

std::uint32_t read_u32_be(const std::byte* data, std::size_t offset) {
    return (static_cast<std::uint32_t>(byte_at(data, offset)) << 24) |
           (static_cast<std::uint32_t>(byte_at(data, offset + 1)) << 16) |
           (static_cast<std::uint32_t>(byte_at(data, offset + 2)) << 8) |
           static_cast<std::uint32_t>(byte_at(data, offset + 3));
}

bool has_bytes(std::size_t size, std::size_t offset, std::size_t count) {
    return offset <= size && count <= size - offset;
}

bool fail(std::string& error_message, const char* message) {
    error_message = message;
    return false;
}

}  // namespace

// RTP Header 解析流程：
// 1. 检查最小长度
// 2. 解析固定 12 字节 Header
// 3. 跳过 CSRC 列表
// 4. 跳过 Header Extension
// 5. 计算 Payload 的位置和长度
// 6. 去掉 Padding
// 7. 返回解析结果
bool parse_rtp_header(const std::byte* data,
                      std::size_t size,
                      RtpHeader& header,
                      std::string& error_message) {
    header = {};
    error_message.clear();

    if (data == nullptr) {
        return fail(error_message, "RTP packet data is null");
    }
    if (size < MinimumRtpHeaderSize) {
        return fail(error_message,
                    "RTP packet is shorter than the minimum header size");
    }

    const auto first = byte_at(data, 0);
    header.version = static_cast<std::uint8_t>(first >> 6);
    header.padding = (first & 0x20U) != 0;
    header.extension = (first & 0x10U) != 0;
    header.csrc_count = static_cast<std::uint8_t>(first & 0x0FU);

    if (header.version != 2) {
        return fail(error_message, "unsupported RTP version");
    }

    const auto second = byte_at(data, 1);
    header.marker = (second & 0x80U) != 0;
    header.payload_type = static_cast<std::uint8_t>(second & 0x7FU);
    header.sequence_number = read_u16_be(data, 2);
    header.timestamp = read_u32_be(data, 4);
    header.ssrc = read_u32_be(data, 8);

    std::size_t offset = MinimumRtpHeaderSize;
    const auto csrc_size = static_cast<std::size_t>(header.csrc_count) * 4;
    if (!has_bytes(size, offset, csrc_size)) {
        return fail(error_message, "RTP CSRC list exceeds packet size");
    }
    offset += csrc_size;

    if (header.extension) {
        constexpr std::size_t ExtensionHeaderSize = 4;
        if (!has_bytes(size, offset, ExtensionHeaderSize)) {
            return fail(error_message,
                        "RTP header extension is missing its length");
        }

        const auto extension_words = read_u16_be(data, offset + 2);
        const auto extension_size =
            static_cast<std::size_t>(extension_words) * 4;
        offset += ExtensionHeaderSize;

        if (!has_bytes(size, offset, extension_size)) {
            return fail(error_message,
                        "RTP header extension exceeds packet size");
        }
        offset += extension_size;
    }

    if (offset > size) {
        return fail(error_message, "RTP header exceeds packet size");
    }

    header.header_size = offset;
    header.payload_offset = offset;
    header.payload_size = size - offset;

    if (header.padding) {
        if (header.payload_size == 0) {
            return fail(error_message, "RTP padding flag set without payload");
        }

        header.padding_size = byte_at(data, size - 1);
        if (header.padding_size == 0 ||
            header.padding_size > header.payload_size) {
            return fail(error_message, "invalid RTP padding size");
        }
        header.payload_size -= header.padding_size;
    }

    return true;
}

}  // namespace streamlens
