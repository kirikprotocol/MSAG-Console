/* ************************************************************************* *
 * Generic FIFO list housekeeping structures.
 * ************************************************************************* */
#ifndef __CORE_BUFFERS_FIFO_LIST_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __CORE_BUFFERS_FIFO_LIST_HPP

namespace smsc {
namespace core {
namespace buffers {

class FifoLink {
protected:
  FifoLink * mPrev; //'mPrev' points to back node.
  FifoLink * mNext; //'mNext' points to front node.

public:
  FifoLink() : mPrev(0), mNext(0)
  { }
  ~FifoLink()
  { }

  bool isLinked(void) const { return mPrev || mNext; }

  //Inserts this node to end of FIFO list.
  void push(FifoLink * & fifomHead, FifoLink * & fifomTail);
  //Excludes this node from start of FIFO list.
  void pop(FifoLink * & fifomHead, FifoLink * & fifomTail);
  //excludes this node from arbitrary position in list.
  void unlink(FifoLink * & fifomHead, FifoLink * & fifomTail);
};

// ------------------------------------------------------------
// Generic list of FIFO-linked elements:
// elements may be added to tail only and removed either from
// head or any intermediate position.
// ------------------------------------------------------------
class FifoList {
protected:
  FifoLink * mHead;
  FifoLink * mTail;

public:
  FifoList() : mHead(0), mTail(0)
  { }
  ~FifoList()
  { }

  bool empty(void) const { return mHead == 0; }

  bool isLinked(const FifoLink & p_node) const
  {
    return p_node.isLinked() || ((const FifoLink *)mHead == &p_node);
  }

  //Extracts element at front of list.
  //Returns NULL if list is empty.
  FifoLink *  pop_front(void);
  //Appends element at to tail of queue.
  void        push_back(FifoLink & use_node);
  //Excludes element from arbitrary position in list.
  //NOTE: element MUST belong to this list!
  void        unlink(FifoLink & p_node) { p_node.unlink(mHead, mTail); }
};

// ---------------------------------------------------------------
// Pure queue of some objects containing FIFO links.
// Eelements may be added to tail only and removed from head only.
// Additionally size of queue is maintained.
// ---------------------------------------------------------------
template <
  class _TArg // : public FifoLink
, typename _SizeTypeArg = unsigned //MUST be an unsigned integer type,
                                   //implicitily restricts capacity of queue
>
class QueueOf_T : protected FifoList {
protected:
  _SizeTypeArg _size;

public:
  typedef _SizeTypeArg size_type;

  QueueOf_T() : FifoList(), _size(0)
  { }
  ~QueueOf_T()
  { }

  bool empty(void) const { return FifoList::empty(); }

  bool isLinked(const _TArg & p_node) const
  {
    return FifoList::isLinked(static_cast<const FifoLink &>(p_node));
  }

  size_type size(void) const { return _size; }

  //Extracts head element.
  //Returns or NULL if queue is empty.
  _TArg * pop_front(void)
  {
    if (_size) {
      --_size;
      return static_cast<_TArg*>(FifoList::pop_front());
    }
    return 0;
  }

  //Appends element to tail of queue.
  //Returns false if queue capacity is already exceeded.
  bool  push_back(_TArg & use_node)
  {
    if (!(++_size)) {
      --_size;
      return false;
    }
    FifoList::push_back(static_cast<FifoLink &>(use_node));
    return true;
  }

  //Excludes element from arbitrary position in queue.
  //NOTE: element MUST belong to this queue!
  bool unlink(_TArg & use_node)
  {
    if (!_size || !isLinked(use_node))
      return false;
    FifoList::unlink(static_cast<FifoLink &>(use_node));
    --_size;
    return true;
  }
};

} //buffers
} //core
} //smsc

#endif /* __CORE_BUFFERS_FIFO_LIST_HPP */

