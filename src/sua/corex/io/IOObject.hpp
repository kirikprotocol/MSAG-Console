#ifndef __COREX_IO_IOOBJECT_HPP__
# define __COREX_IO_IOOBJECT_HPP__ 1

# include <string>

namespace corex {
namespace io {

template <class LOCK>
class IOObjectsPool_tmpl;

class InputStream;
class OutputStream;

class IOObject {
public:
  virtual ~IOObject() {}

  virtual void connect() = 0;
  virtual void close() = 0;
  virtual void setNonBlocking(bool on) = 0;

  virtual InputStream* getInputStream() = 0;
  virtual OutputStream* getOutputStream() = 0;

  virtual std::string toString() const = 0;

protected:
  int getDescriptor();
  virtual int _getDescriptor() = 0;
  template <class LOCK>
  friend class IOObjectsPool_tmpl; // to grant access to getDescriptor()

  void setNonBlocking(int fd, bool on);
};

}}

#endif
