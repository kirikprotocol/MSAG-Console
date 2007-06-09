#ifndef __bdb_agent_objqueue_hpp__
# define __bdb_agent_objqueue_hpp__ 1

# include <deque>
# include <pthread.h>
# include <util/Exception.hpp>

namespace smsc {
namespace util {
namespace comm_comp {

class QueueCongestionException : public smsc::util::Exception {
public:
  QueueCongestionException() : Exception() {}
  QueueCongestionException(const char* fmt, ...)
    : Exception() { SMSC_UTIL_EX_FILL(fmt); }
};

/*
** Abstraction for Objects Queue working in environment with mulitple producers/multiple consumers.
*/
template <class T, int QUEUE_LIMIT=10>
class ObjQueue {
public:
  ObjQueue() {
    pthread_mutex_init(&_lock, NULL);
    pthread_cond_init(&_signal, NULL);
  }

  ~ObjQueue() {
    pthread_mutex_destroy(&_lock);
    pthread_cond_destroy(&_signal);
  }

  /*
  ** put copy of object to queue
  */
  void push (const T& element) {
    try {
      pthread_mutex_lock(&_lock);

      size_t curQueueSize = _innerQue.size();
      if ( curQueueSize > QUEUE_LIMIT )
        throw QueueCongestionException("ObjQueue::push::: current queue size=[%d] exceeded max. queue limit size=[%d]", curQueueSize, QUEUE_LIMIT);

      _innerQue.push_back(element);
      if ( curQueueSize == 0 ) pthread_cond_signal(&_signal);
      pthread_mutex_unlock(&_lock);
    } catch (...)  {
      pthread_mutex_unlock(&_lock);
      throw;
    }
  }

  /*
  ** get copy of object from queue
  */
  T front() {
    try {
      pthread_mutex_lock(&_lock);
      if ( _innerQue.size() == 0 ) pthread_cond_wait(&_signal, &_lock);
      T firstElement;
      if ( _innerQue.size() != 0 ) {
        firstElement = _innerQue.front();
        _innerQue.pop_front();
      }
      pthread_mutex_unlock(&_lock);
      return firstElement;
    } catch (...) {
      pthread_mutex_unlock(&_lock);
      throw;
    }
  }
private:
  pthread_mutex_t _lock;
  pthread_cond_t _signal;

  std::deque<T> _innerQue;
};

}}}

#endif
