#pragma once

#include <cstddef>
#include <string>

#include "streamlens/rtp/rtp_header.h"

namespace streamlens {

bool parse_rtp_header(const std::byte* data,
                      std::size_t size,
                      RtpHeader& header,
                      std::string& error_message);

}  // namespace streamlens
