/* ************************************************************************* *
 * FIFO queue template.
 * ************************************************************************* */
#ifndef __EYELINE_UTIL_FIFO_QUEUE_HPP
#ident "@(#)$Id$"
#define __EYELINE_UTIL_FIFO_QUEUE_HPP

namespace eyeline {
namespace util {

class FifoLink {
protected:
  FifoLink * _prev; //'_prev' points to back node.
  FifoLink * _next; //'_next' points to front node.

public:
  FifoLink() : _prev(0), _next(0)
  { }
  ~FifoLink()
  { }

  bool isLinked(void) const { return _prev || _next; }

  //Inserts this node to end of FIFO queue.
  void push(FifoLink * & fifo_head, FifoLink * & fifo_tail)
  {
    if (fifo_tail)
      fifo_tail->_prev = this;
    _next = fifo_tail;
    fifo_tail = this;
    if (!fifo_head)
      fifo_head = this;
  }
  //Excludes this node from start of FIFO queue.
  void pop(FifoLink * & fifo_head, FifoLink * & fifo_tail)
  {
    fifo_head = _prev;
    _prev = 0;
    if (!fifo_head)
      fifo_tail = NULL;
  }
  //excludes this node from queue
  void unlink(FifoLink * & fifo_head, FifoLink * & fifo_tail)
  {
    if (!_next) { //front element
      pop(fifo_head, fifo_tail);
      return;
    }
    if (!_prev) { //back element
      fifo_tail = _next;
      _next = 0;
      if (!fifo_tail)
        fifo_head = NULL;
      return;
    }
    //intermediate element
    _next->_prev = _prev;
    _prev->_next = _next;
  }
};

class FifoQueue {
  FifoLink * _head;
  FifoLink * _tail;

public:
  FifoQueue() : _head(0), _tail(0)
  { }
  ~FifoQueue()
  { }

  bool empty(void) const { return _head == 0; }

  bool isLinked(const FifoLink * p_node) const
  {
    return p_node->isLinked() || ((const FifoLink *)_head == p_node);
  }

  FifoLink * pop_front(void)
  {
    FifoLink * pNode = _head;
    if (_head)
      pNode->pop(_head, _tail);
    if (!_head)
      _tail = 0;
    return pNode;
  }
  void push_back(FifoLink * use_node)
  {
    if (_tail)
      use_node->push(_head, _tail);
    _tail = use_node;
    if (!_head)
      _head = use_node;
  }

  void unlink(FifoLink * p_node)
  {
    p_node->unlink(_head, _tail);
  }
};

} //util
} //eyeline

#endif /* __EYELINE_UTIL_FIFO_QUEUE_HPP */

