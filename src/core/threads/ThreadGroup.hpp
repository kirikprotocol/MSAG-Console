#ifndef __CORE_THREADS_THREADGROUP_HPP__
# define __CORE_THREADS_THREADGROUP_HPP__

# include <vector>
# include "core/threads/Thread.hpp"
# include "core/buffers/CyclicQueue.hpp"
# include "core/synchronization/Mutex.hpp"
# include "core/synchronization/EventMonitor.hpp"

namespace smsc {
namespace core {
namespace threads {

/*
  class ThreadGroup implements group of running threads each of which has
  dedicated event's queue. ThreadGroup designed to work in the environment
  with one events Producer and many events Consumers. Threads running under
  ThreadGroup control is Consumer threads. Producer must be only one.

  Derived class that implements event processing behaviour (i.e. that is a Consumer)
  must implements abstract methods isRunning() and processEvent(EVENT* event).
*/
template<class EVENT>
class ThreadGroup
{
public:
  ThreadGroup(unsigned arg_disp_threads_count)
  {
    for(int i=0;i<arg_disp_threads_count;i++)
      _dispThreads.push_back(new DispatcherRunner(this,i));

    _lastDispIdx=0;
  }
  virtual ~ThreadGroup()
  {
    std::vector<DispatcherRunner*>::size_type count=_dispThreads.size();
    for(std::vector<DispatcherRunner*>::size_type i=0; i<count; i++)
      delete _dispThreads[i];
  }

  void start()
  {
    for (std::vector<DispatcherRunner*>::iterator iter=_dispThreads.begin(), end_iter=_dispThreads.end();
         iter != end_iter; ++iter)
      (*iter)->Start();
  }

  void stop()
  {
    std::vector<DispatcherRunner*>::size_type count=_dispThreads.size();
    for(std::vector<DispatcherRunner*>::size_type i=0; i<count; i++)
    {
      _dispThreads[i]->mon.notify();
      _dispThreads[i]->WaitFor();
    }
  }

  void eventDispatcher(int idx)
  {
    EventQueue& queue=_dispThreads[idx]->queue;
    synchronization::EventMonitor& mon=_dispThreads[idx]->mon;

    while(isRunning())
    {
      EVENT* event;
      {
        synchronization::MutexGuard mg(mon);
        if(queue.Count()==0)
        {
          mon.wait(100);
          continue;
        }
        queue.Pop(event);
      }
      processEvent(event);
    }
  }

protected:
  void publishEvent(EVENT *event)
  {
    {
      synchronization::MutexGuard mg(_dispThreads[_lastDispIdx]->mon);
      _dispThreads[_lastDispIdx]->queue.Push(event);
      _dispThreads[_lastDispIdx]->mon.notify();
    }
    _lastDispIdx=(unsigned)((_lastDispIdx+1) % _dispThreads.size());
  }

  virtual bool isRunning() = 0;
  virtual void processEvent(EVENT* event) = 0;

private:
  typedef buffers::CyclicQueue<EVENT*> EventQueue;

  class DispatcherRunner : public Thread {
  public:
    ThreadGroup<EVENT>* listener;
    int idx;
    EventQueue queue;
    core::synchronization::EventMonitor mon;
    DispatcherRunner(ThreadGroup<EVENT>* arg_listener,int arg_idx)
    : listener(arg_listener),idx(arg_idx)
    {}
    int Execute()
    {
      listener->eventDispatcher(idx);
      return 0;
    }
  };

  friend class DispatcherRunner; //this is portable way for granting access to EventQueue from inner class
  std::vector<DispatcherRunner*> _dispThreads;
  unsigned _lastDispIdx;
};

}}}

#endif
