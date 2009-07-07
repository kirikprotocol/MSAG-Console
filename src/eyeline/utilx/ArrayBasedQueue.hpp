#ifndef __EYELINE_UTILX_ARRAYBASEDQUEUE_HPP__
# define __EYELINE_UTILX_ARRAYBASEDQUEUE_HPP__

# include <sys/types.h>

# include "eyeline/utilx/Exception.hpp"
# include "core/synchronization/Mutex.hpp"
# include "core/synchronization/MutexGuard.hpp"

namespace eyeline {
namespace utilx {

template <class ELEMENT, class LOCK=smsc::core::synchronization::Mutex>
class ArrayBasedQueue
{
public:
  explicit ArrayBasedQueue(size_t maxElementsAtQueue)
    : _maxElementsAtQueue(maxElementsAtQueue),
      _head(0), _tail(0) {
    _buffer = new ELEMENT[maxElementsAtQueue];
  }

  ~ArrayBasedQueue() { delete [] _buffer; }

  void enqueue(const ELEMENT& element) {
    smsc::core::synchronization::MutexGuardTmpl<LOCK> synchronize(_lock);
    if ( _head == _tail + 1 )
      throw utilx::QueueOverflowException("Queue<ELEMENT>::push::: queue is full");
    else {
      _buffer[_tail++] = element;
      if ( _tail == _maxElementsAtQueue )
        _tail = 0;
    }
  }

  ELEMENT dequeue() {
    smsc::core::synchronization::MutexGuardTmpl<LOCK> synchronize(_lock);
    if ( _head == _tail )
      throw utilx::QueueUnderflowException("Queue<ELEMENT>::pop::: queue is empty");
    else {
      ELEMENT element = _buffer[_head++];
      if ( _head == _maxElementsAtQueue )
        _head = 0;

      return element;
    }
  }

  bool isEmpty() const {
    smsc::core::synchronization::MutexGuardTmpl<LOCK> synchronize(_lock);
    return _head == _tail;
  }

private:
  size_t _maxElementsAtQueue;
  unsigned _head, _tail;
  ELEMENT* _buffer;

  mutable LOCK _lock;
};

}}

#endif
