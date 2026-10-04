#include "streamlens/statistics/stream_statistics.h"

namespace streamlens {

void StreamStatistics::reset() noexcept {
    packet_count_ = 0;
    byte_count_ = 0;
    lost_packet_count_ = 0;
    has_sequence_ = false;
    highest_sequence_number_ = 0;
    payload_type_ = 0;
    ssrc_ = 0;
}

// 更新一条 RTP 包的统计信息：
// 1. 累加包数量和字节数
// 2. 记录 Payload Type 和 SSRC
// 3. 根据 Sequence Number 检测丢包
// 4. 正确处理 16 位 Sequence Number 回绕
// 5. 忽略重复包和乱序旧包对丢包数的影响
void StreamStatistics::update(const RtpHeader& header,
                              std::size_t packet_size) noexcept {
    ++packet_count_;
    byte_count_ += static_cast<std::uint64_t>(packet_size);
    payload_type_ = header.payload_type;
    ssrc_ = header.ssrc;

    if (!has_sequence_) {
        has_sequence_ = true;
        highest_sequence_number_ = header.sequence_number;
        return;
    }

    // RTP 序列号是 16 位，会从 65535 回绕到 0。
    const auto forward_distance = static_cast<std::uint16_t>(
        header.sequence_number - highest_sequence_number_);

    // 小于序列号范围一半的距离表示序列向前推进。
    // 旧包或重复包不会增加丢包数量。
    if (forward_distance > 0 && forward_distance < 0x8000U) {
        lost_packet_count_ += static_cast<std::uint64_t>(forward_distance - 1);
        highest_sequence_number_ = header.sequence_number;
    }
}

std::uint64_t StreamStatistics::packet_count() const noexcept {
    return packet_count_;
}

std::uint64_t StreamStatistics::byte_count() const noexcept {
    return byte_count_;
}

std::uint64_t StreamStatistics::lost_packet_count() const noexcept {
    return lost_packet_count_;
}

double StreamStatistics::loss_rate() const noexcept {
    const auto expected_packet_count = packet_count_ + lost_packet_count_;
    if (expected_packet_count == 0) {
        return 0.0;
    }
    return static_cast<double>(lost_packet_count_) * 100.0 /
           static_cast<double>(expected_packet_count);
}

bool StreamStatistics::has_sequence() const noexcept {
    return has_sequence_;
}

std::uint16_t StreamStatistics::highest_sequence_number() const noexcept {
    return highest_sequence_number_;
}

std::uint8_t StreamStatistics::payload_type() const noexcept {
    return payload_type_;
}

std::uint32_t StreamStatistics::ssrc() const noexcept {
    return ssrc_;
}

}  // namespace streamlens
