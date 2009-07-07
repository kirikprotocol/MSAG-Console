#ifndef __EYELINE_COREX_IO_IOOBJECT_HPP__
# define __EYELINE_COREX_IO_IOOBJECT_HPP__

# include <string>
# include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace corex {
namespace io {

template <class LOCK>
class IOObjectsPool_tmpl;

class InputStream;
class OutputStream;

class IOObject {
public:
  virtual ~IOObject() {}

  virtual void close() = 0;
  virtual void setNonBlocking(bool on) = 0;

  virtual InputStream* getInputStream() const = 0;
  virtual OutputStream* getOutputStream() const = 0;

  virtual std::string toString() const = 0;

  const std::string& getId() const;
  void setId(const std::string& id);
protected:
  int getDescriptor();
  virtual int _getDescriptor() = 0;
  template <class LOCK>
  friend class IOObjectsPool_tmpl; // to grant access to getDescriptor()

  void setNonBlocking(int fd, bool on);
private:
  std::string _id;
};

}}}

#endif
