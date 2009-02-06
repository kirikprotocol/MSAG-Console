#ifndef __SMPPDMPLX_NETWORKEXCEPTION_HPP__
# define __SMPPDMPLX_NETWORKEXCEPTION_HPP__

# include <util/Exception.hpp>

namespace smpp_dmplx {

class NetworkException : public smsc::util::SystemError
{
public:
  explicit NetworkException(const char* where) : SystemError(where) {}
};

class EOFException : public NetworkException
{
public:
  explicit EOFException(const char* where) : NetworkException(where) {}
};

}

#endif
