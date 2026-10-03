#include "streamlens/network/udp_receiver.h"

#include <cerrno>
#include <cstring>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace streamlens {
namespace {

constexpr int InvalidSocket = -1;
int last_socket_error() { return errno; }
void close_socket(int socket) { ::close(socket); }

std::string socket_error_message(const char* operation, int error_code) {
    return std::string(operation) + " failed (error " +
           std::to_string(error_code) + ")";
}

}  // namespace

UdpReceiver::~UdpReceiver() {
    close();
}

bool UdpReceiver::open(std::uint16_t port, std::string& error_message) {
    close();

    // 调用全局命名空间中的 socket 函数。前面的 :: 表示“从全局作用域查找”。
    socket_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_ == InvalidSocket) {
        error_message = socket_error_message("socket", last_socket_error());
        return false;
    }

    int reuse_address = 1;
    if (::setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR,
                     &reuse_address, sizeof(reuse_address)) < 0) {
        error_message = socket_error_message("setsockopt", last_socket_error());
        close();
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    if (::bind(socket_, reinterpret_cast<const sockaddr*>(&address),
               sizeof(address)) < 0) {
        error_message = socket_error_message("bind", last_socket_error());
        close();
        return false;
    }

    timeval timeout{};
    timeout.tv_sec = 0;
    timeout.tv_usec = 200000;
    ::setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO,
                 reinterpret_cast<const char*>(&timeout), sizeof(timeout));

    return true;
}

int UdpReceiver::receive(void* buffer,
                         std::size_t capacity,
                         std::string& error_message) {
    if (!is_open()) {
        error_message = "receiver is not open";
        return -1;
    }

    const auto received = ::recvfrom(
        socket_, buffer, capacity, 0,
        nullptr, nullptr);

    if (received >= 0) {
        return received;
    }

    const int error_code = last_socket_error();
    if (error_code == EAGAIN || error_code == EWOULDBLOCK || error_code == EINTR) {
        return 0;
    }

    error_message = socket_error_message("recvfrom", error_code);
    return -1;
}

void UdpReceiver::close() {
    if (is_open()) {
        close_socket(socket_);
        socket_ = InvalidSocket;
    }
}

bool UdpReceiver::is_open() const noexcept {
    return socket_ != InvalidSocket;
}

}  // namespace streamlens
