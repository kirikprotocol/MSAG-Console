#ifndef __SMPPDMPLX_NETWORKEXCEPTION_HPP__
# define __SMPPDMPLX_NETWORKEXCEPTION_HPP__ 1

# include <util/Exception.hpp>

//# include <core_ax/network/Socket.hpp>

namespace smpp_dmplx {

class EOFException : public smsc::util::Exception
{
public:
  EOFException(const char* where) : Exception(where) {}
};

}

#endif
