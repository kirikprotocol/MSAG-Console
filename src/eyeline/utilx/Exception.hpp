#ifndef __EYELINE_UTILX_EXCEPTION_HPP__
# define __EYELINE_UTILX_EXCEPTION_HPP__

# include <util/Exception.hpp>

namespace eyeline {
namespace utilx {

class FieldNotSetException : public smsc::util::Exception {
public:
  FieldNotSetException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

class CongestionException : public smsc::util::Exception {
public:
  CongestionException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

class InterruptedException : public smsc::util::Exception {
public:
  InterruptedException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

class UnsupportedOperationException : public smsc::util::Exception {
public:
  UnsupportedOperationException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

class DeserializationException : public smsc::util::Exception {
public:
  DeserializationException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

class SerializationException : public smsc::util::Exception {
public:
  SerializationException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

class QueueOverflowException : public smsc::util::Exception {
public:
  QueueOverflowException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

class QueueUnderflowException : public smsc::util::Exception {
public:
  QueueUnderflowException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

class ProtocolException : public smsc::util::Exception {
public:
  ProtocolException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

class RegistryKeyNotFound : public smsc::util::Exception {
public:
  RegistryKeyNotFound(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

}}

#endif
