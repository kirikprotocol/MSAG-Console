#ifndef __CORE_THREADS_THREADEDTASK_HPP__
#ifndef __GNUC__
#ident "$Id$"
#endif
#define __CORE_THREADS_THREADEDTASK_HPP__

namespace smsc{
namespace core{
namespace threads{

class ThreadedTask {
public:
  ThreadedTask(bool del_on_completion = true)
    : isStopping(false), isReleased(false), delTask(del_on_completion)
  { }
  virtual ~ThreadedTask()
  { }

  //set task completion mode: deleting or callback calling
  void setDelOnCompletion(bool del_task = true) { delTask = del_task; }
  //tells whether the task destructor or onRelease() callback should be called on completion
  bool delOnCompletion(void) const { return delTask; }

  bool stopping(void) const { return isStopping; }

  // -------------------------------------------
  // -- ThreadedTask interface methods
  // -------------------------------------------
  virtual int Execute(void) = 0;
  virtual const char* taskName(void) = 0; //TODO: mark this as const

  virtual void stop(void) { isStopping = true; }
  virtual void onRelease(void) { isReleased = true; }

protected:
  volatile bool isStopping;
  volatile bool isReleased;

private:
  bool          delTask; //indicates that task should be deleted on completion
};

}//threads
}//core
}//smsc

#endif /* __CORE_THREADS_THREADEDTASK_HPP__ */

