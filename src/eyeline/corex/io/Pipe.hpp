#ifndef __EYELINE_COREX_IO_PIPE_HPP__
# define __EYELINE_COREX_IO_PIPE_HPP__

# include <util/Exception.hpp>
# include <eyeline/corex/io/IOStreams.hpp>
# include <eyeline/corex/io/IOObject.hpp>
# include <logger/Logger.h>

namespace eyeline {
namespace corex {
namespace io {

class Pipe {
public:
  virtual ~Pipe() {}

  virtual IOObject* getLeftSide() = 0;
  virtual IOObject* getRightSide() = 0;

  virtual void close() = 0;

  virtual std::string toString() const = 0;
};

class UnnamedPipe : public Pipe {
public:
  UnnamedPipe();
  virtual ~UnnamedPipe();

  virtual IOObject* getLeftSide();
  virtual IOObject* getRightSide();

  virtual void close();

  virtual std::string toString() const;
private:
  UnnamedPipe(const UnnamedPipe& rhs);
  UnnamedPipe& operator=(const UnnamedPipe& rhs);

  class PipeEnd : public IOObject {
  public:
    PipeEnd(UnnamedPipe* creator, int fd);
    virtual ~PipeEnd();
    virtual void connect();
    virtual void close();

    using IOObject::setNonBlocking;

    virtual void setNonBlocking(bool on);

    virtual InputStream* getInputStream();
    virtual OutputStream* getOutputStream();

    virtual std::string toString() const;
  protected:
    virtual int _getDescriptor();

  private:
    PipeEnd(const PipeEnd& rhs);
    PipeEnd& operator=(const PipeEnd& rhs);

    UnnamedPipe* _creator;
    int _fd;
    GenericInputStream _inputStream;
    GenericOutputStream _outputStream;
  };

  void halfClose(int fd);
  friend class PipeEnd; // to grant access to halfClose
  int _fds[2];
  PipeEnd* _leftSide;
  PipeEnd* _rightSide;
};

}}}
#endif
