#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include <sys/socket.h>

namespace streamlens {

class UdpSender {
public:
    UdpSender() = default;
    ~UdpSender();

    UdpSender(const UdpSender&) = delete;
    UdpSender& operator=(const UdpSender&) = delete;

    bool open(const std::string& host,
              std::uint16_t port,
              std::string& error_message);

    // Returns the sent byte count, or -1 on error.
    int send(const void* data,
             std::size_t size,
             std::string& error_message);

    void close();
    bool is_open() const noexcept;

private:
    int socket_{-1};
    sockaddr_storage destination_{};
    socklen_t destination_length_{0};
};

}  // namespace streamlens
