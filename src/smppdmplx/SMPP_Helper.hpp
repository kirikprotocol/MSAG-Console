#ifndef __SMPPDMPLX_SMPP_HELPER_HPP__
# define __SMPPDMPLX_SMPP_HELPER_HPP__

# include <string>
# include <sys/types.h>
# include <memory>
# include <vector>

# include <smppdmplx/BufferedInputStream.hpp>
# include <smppdmplx/BufferedOutputStream.hpp>
# include <smppdmplx/SMPP_message.hpp>

namespace smpp_dmplx {

class SMPP_Helper {
public:
  static void readCString(BufferedInputStream& sourceBuf, std::string& value, size_t maxSize);
  static void writeCString(BufferedOutputStream& destBuf, const std::string& value, size_t maxSize);
  static std::string hexdmp(const std::vector<uint8_t>& buf);
};

}

#endif
