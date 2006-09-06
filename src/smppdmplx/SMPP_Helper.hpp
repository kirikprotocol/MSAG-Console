#ifndef __SMPPDMPLX_SMPP_HELPER_HPP__
# define __SMPPDMPLX_SMPP_HELPER_HPP__ 1

# include <string>
# include <sys/types.h>
# include <memory>
# include <vector>
# include "BufferedInputStream.hpp"
# include "BufferedOutputStream.hpp"
# include "SMPP_message.hpp"

namespace smpp_dmplx {

class SMPP_Helper {
public:
  static void readCString(smpp_dmplx::BufferedInputStream& sourceBuf, std::string& value, size_t maxSize);
  static void writeCString(smpp_dmplx::BufferedOutputStream& destBuf, const std::string& value, size_t maxSize);
  static std::string hexdmp(const std::vector<uint8_t>& buf);
};

}

#endif
