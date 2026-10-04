#include <cassert>
#include <cstddef>
#include <cstdint>

#include "streamlens/statistics/stream_statistics.h"

namespace {

streamlens::RtpHeader header(std::uint16_t sequence) {
    streamlens::RtpHeader result;
    result.sequence_number = sequence;
    result.payload_type = 96;
    result.ssrc = 0x12345678;
    return result;
}

void test_packet_and_byte_counts() {
    streamlens::StreamStatistics statistics;
    statistics.update(header(100), 1200);
    statistics.update(header(101), 1300);

    assert(statistics.packet_count() == 2);
    assert(statistics.byte_count() == 2500);
    assert(statistics.lost_packet_count() == 0);
    assert(statistics.payload_type() == 96);
    assert(statistics.ssrc() == 0x12345678);
}

void test_missing_packets() {
    streamlens::StreamStatistics statistics;
    statistics.update(header(100), 100);
    statistics.update(header(101), 100);
    statistics.update(header(102), 100);
    statistics.update(header(105), 100);

    assert(statistics.packet_count() == 4);
    assert(statistics.lost_packet_count() == 2);
    assert(statistics.loss_rate() > 33.33);
    assert(statistics.loss_rate() < 33.34);
}

void test_sequence_wraparound() {
    streamlens::StreamStatistics statistics;
    statistics.update(header(65534), 100);
    statistics.update(header(65535), 100);
    statistics.update(header(0), 100);
    statistics.update(header(1), 100);

    assert(statistics.packet_count() == 4);
    assert(statistics.lost_packet_count() == 0);
    assert(statistics.highest_sequence_number() == 1);
}

void test_duplicate_and_old_packets() {
    streamlens::StreamStatistics statistics;
    statistics.update(header(100), 100);
    statistics.update(header(102), 100);
    statistics.update(header(101), 100);
    statistics.update(header(102), 100);

    assert(statistics.packet_count() == 4);
    assert(statistics.lost_packet_count() == 1);
    assert(statistics.highest_sequence_number() == 102);
}

}  // namespace

int main() {
    test_packet_and_byte_counts();
    test_missing_packets();
    test_sequence_wraparound();
    test_duplicate_and_old_packets();
    return 0;
}
