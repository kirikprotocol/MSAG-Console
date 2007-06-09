#ifndef __BDB_AGENT_WORKERSPOOL_HPP__
# define __BDB_AGENT_WORKERSPOOL_HPP__ 1

# include <core/threads/ThreadedTask.hpp>
# include <core/threads/ThreadPool.hpp>

namespace smsc {
namespace util {
namespace comm_comp {

class Worker : public smsc::core::threads::ThreadedTask {
public:
  Worker() {}
  virtual int Execute();
  virtual const char* taskName() { return "Worker"; }
private:
  Worker(const Worker& rhs);
  Worker& operator=(const Worker& rhs);
};

/*
** Proxy to ThreadPool class with prior generation of Workers objects.
*/
class WorkersPool {
public:
  WorkersPool(int workersNum=10);

  ~WorkersPool();

  void start();

  void finish();
private:
  smsc::core::threads::ThreadPool _workers;
  int _workersNum;
  bool _wasFinished;
  WorkersPool(const WorkersPool& rhs);
  WorkersPool& operator=(const WorkersPool& rhs);
};


}}}

#endif
