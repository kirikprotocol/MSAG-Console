/* ************************************************************************* *
 * FIFO queue housekeeping classes.
 * ************************************************************************* */
#ifndef __SMSC_UTIL_FIFO_QUEUE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_UTIL_FIFO_QUEUE_HPP

namespace smsc {
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
  void push(FifoLink * & fifo_head, FifoLink * & fifo_tail);
  //Excludes this node from start of FIFO queue.
  void pop(FifoLink * & fifo_head, FifoLink * & fifo_tail);
  //excludes this node from queue
  void unlink(FifoLink * & fifo_head, FifoLink * & fifo_tail);
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

  FifoLink *  pop_front(void);
  void        push_back(FifoLink * use_node);

  void unlink(FifoLink * p_node)
  {
    p_node->unlink(_head, _tail);
  }
};

} //util
} //smsc

#endif /* __SMSC_UTIL_FIFO_QUEUE_HPP */

