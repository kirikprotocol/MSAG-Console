#ifndef __COREX_IO_PIPE_HPP__
# define __COREX_IO_PIPE_HPP__ 1

# include <util/Exception.hpp>
# include <sua/corex/io/IOStreams.hpp>
# include <sua/corex/io/IOObject.hpp>
# include <logger/Logger.h>

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


// : public IOObject
}}
#endif
