#include "streamlens/network/udp_sender.h"

#include <cerrno>
#include <cstring>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

namespace streamlens {
namespace {

constexpr int InvalidSocket = -1;

std::string socket_error_message(const char* operation, int error_code) {
    return std::string(operation) + " failed (error " +
           std::to_string(error_code) + ")";
}

}  // namespace

UdpSender::~UdpSender() {
    close();
}

bool UdpSender::open(const std::string& host,
                     std::uint16_t port,
                     std::string& error_message) {
    close();

    const std::string port_text = std::to_string(port);
    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    addrinfo* results = nullptr;
    const int resolve_result = ::getaddrinfo(
        host.c_str(), port_text.c_str(), &hints, &results);
    if (resolve_result != 0) {
        error_message = "getaddrinfo failed: " +
                        std::string(gai_strerror(resolve_result));
        return false;
    }

    socket_ = ::socket(results->ai_family,
                       results->ai_socktype,
                       results->ai_protocol);
    if (socket_ == InvalidSocket) {
        error_message = socket_error_message("socket", errno);
        ::freeaddrinfo(results);
        return false;
    }

    if (results->ai_addrlen > sizeof(destination_)) {
        error_message = "destination address is too large";
        ::close(socket_);
        socket_ = InvalidSocket;
        ::freeaddrinfo(results);
        return false;
    }

    std::memcpy(&destination_, results->ai_addr, results->ai_addrlen);
    destination_length_ = results->ai_addrlen;
    ::freeaddrinfo(results);
    return true;
}

int UdpSender::send(const void* data,
                    std::size_t size,
                    std::string& error_message) {
    if (!is_open()) {
        error_message = "sender is not open";
        return -1;
    }

    const auto sent = ::sendto(
        socket_, data, size, 0,
        reinterpret_cast<const sockaddr*>(&destination_),
        destination_length_);

    if (sent < 0) {
        error_message = socket_error_message("sendto", errno);
        return -1;
    }

    return sent;
}

void UdpSender::close() {
    if (is_open()) {
        ::close(socket_);
        socket_ = InvalidSocket;
        destination_ = {};
        destination_length_ = 0;
    }
}

bool UdpSender::is_open() const noexcept {
    return socket_ != InvalidSocket;
}

}  // namespace streamlens
