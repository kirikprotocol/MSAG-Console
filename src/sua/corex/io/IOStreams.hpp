#ifndef __COREX_IO_IOSTREAM_HPP__
# define __COREX_IO_IOSTREAM_HPP__ 1

# include <sys/types.h>
# include <sys/uio.h>
# include <sua/corex/io/IOExceptions.hpp>

namespace corex {
namespace io {

class IOObject;

class InputStream {
public:
  virtual ~InputStream() {}
  virtual ssize_t read(uint8_t *buf, size_t bufSz) = 0;
  virtual IOObject* getOwner() = 0;
};

class OutputStream {
public:
  virtual ~OutputStream() {}
  virtual ssize_t write(const uint8_t *buf, size_t bufSz, uint16_t streamNo=0, bool ordered=false) const = 0;
  virtual ssize_t writev(const struct iovec *iov, int iovcnt) const = 0;
  virtual IOObject* getOwner() = 0;
};

class GenericInputStream : public InputStream {
public:
  GenericInputStream(IOObject* owner, int fd);
  virtual ssize_t read(uint8_t *buf, size_t bufSz);
  virtual IOObject* getOwner();
private:
  IOObject* _owner;
  int _fd;
};

class GenericOutputStream : public OutputStream {
public:
  GenericOutputStream(IOObject* owner, int fd);
  virtual ssize_t write(const uint8_t *buf, size_t bufSz, uint16_t streamNo=0, bool ordered=false) const;
  virtual ssize_t writev(const struct iovec *iov, int iovcnt) const;
  virtual IOObject* getOwner();
private:
  IOObject* _owner;
  int _fd;
};

}}

#endif
