#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace streamlens {

class UdpReceiver {
public:
    UdpReceiver() = default;
    ~UdpReceiver();

    UdpReceiver(const UdpReceiver&) = delete;
    UdpReceiver& operator=(const UdpReceiver&) = delete;

    bool open(std::uint16_t port, std::string& error_message);

    // Returns the received byte count, 0 on timeout, or -1 on error.
    int receive(void* buffer, std::size_t capacity, std::string& error_message);

    void close();
    bool is_open() const noexcept;

private:
    int socket_{-1};
};

}  // namespace streamlens
