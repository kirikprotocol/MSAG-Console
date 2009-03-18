#ifndef __EYELINE_UTILX_BOUNDEDQUEUE_HPP__
# define __EYELINE_UTILX_BOUNDEDQUEUE_HPP__

# include <sys/types.h>
# include <util/Exception.hpp>

namespace eyeline {
namespace utilx {

template <class T>
class BoundedQueue {
public:
  explicit BoundedQueue(size_t qsz)
    : _head(0), _tail(0), _qsz(qsz) {
    _qArray = new T[qsz+1];
    if ( !_qArray ) throw smsc::util::SystemError("BoundedQueue<T>::BoundedQueue::: can't allocate memory");
  }

  ~BoundedQueue() { delete [] _qArray; }

  bool enqueue(const T& element) {
    if ( (_tail +1) % (_qsz+1) == _head )
      return false;
    
    _qArray[_tail] = element;

    _tail = _tail % (_qsz+1) + 1;
    return true;
  }

  bool dequeue(T& element) {
    if ( _head == _tail ) return false;
    element = _qArray[_head];
    _qArray[_head].~T();
    _head = _head % (_qsz + 1) + 1;
    return true;
  }

  size_t getSize() const { return _qsz; }

  bool isEmpty() const { return _head == _tail; }

private:
  T* _qArray;
  size_t _head, _tail, _qsz;
};

template <class T>
class BoundedQueue<T*> {
public:
  explicit BoundedQueue(size_t qsz)
    : _head(0), _tail(0), _qsz(qsz) {
    _qArray = new T*[qsz+1];
    if ( !_qArray ) throw smsc::util::SystemError("BoundedQueue<T*>::BoundedQueue::: can't allocate memory");
  }

  ~BoundedQueue() { delete [] _qArray; }

  bool enqueue(T* element) {
    if ( (_tail +1) % (_qsz+1) == _head )
      return false;
    
    _qArray[_tail] = element;

    _tail = _tail % (_qsz+1) + 1;
    return true;
  }

  bool dequeue(T*& element) {
    if ( _head == _tail ) return false;
    element = _qArray[_head];
    _head = _head % (_qsz + 1) + 1;
    return true;
  }

  size_t getSize() const { return _qsz; }

  bool isEmpty() const { return _head == _tail; }

private:
  T** _qArray;
  size_t _head, _tail, _qsz;
};

}}

#endif
