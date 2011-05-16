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

// ------------------------------------------------------------
// Generic list of FIFO-linked elements:
// elements may be added to tail only and removed either from
// head or any intermediate position.
// ------------------------------------------------------------
class FifoList {
protected:
  FifoLink * _head;
  FifoLink * _tail;

public:
  FifoList() : _head(0), _tail(0)
  { }
  ~FifoList()
  { }

  bool empty(void) const { return _head == 0; }

  bool isLinked(const FifoLink * p_node) const
  {
    return p_node->isLinked() || ((const FifoLink *)_head == p_node);
  }

  //Takes element at head of queue.
  FifoLink *  pop_front(void);

  //Appends element at to tail of queue.
  void        push_back(FifoLink * use_node);

  //Excludes element from queue.
  //NOTE: element MUST belong to this queue
  void unlink(FifoLink * p_node)
  {
    p_node->unlink(_head, _tail);
  }
};

// ------------------------------------------------------------
// Pure FIFO queue:  elements may be added to tail only
// and removed from head only. Additionally size of queue is
// maintained.
// ------------------------------------------------------------
template <
  class _TArg // : public FifoLink
, typename _SizeTypeArg = unsigned //MUST be an unsigned integer type,
                                   //implicitily restricts capacity of queue
>
class FifoQueue_T : protected FifoList {
protected:
  _SizeTypeArg _size;

public:
  typedef _SizeTypeArg size_type;

  FifoQueue_T() : FifoList(), _size(0)
  { }
  ~FifoQueue_T()
  { }

  bool empty(void) const { return FifoList::empty(); }

  bool isLinked(const _TArg * p_node) const
  {
    return FifoList::isLinked(p_node);
  }

  size_type size(void) const { return _size; }

  //Returns head element? or NULL if queue is empty
  _TArg *  pop_front(void)
  {
    if (_size) {
      --_size;
      return static_cast<_TArg*>(FifoList::pop_front());
    }
    return 0;
  }

  //Appends element to tail of queue.
  //Returns false if queue capacity is already exceeded.
  bool push_back(_TArg * use_node)
  {
    if (!(++_size)) {
      --_size;
      return false;
    }
    FifoList::push_back(use_node);
    return true;
  }
};


} //util
} //smsc

#endif /* __SMSC_UTIL_FIFO_QUEUE_HPP */

