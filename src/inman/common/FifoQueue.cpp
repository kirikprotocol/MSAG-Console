#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/common/FifoQueue.hpp"

namespace smsc   {
namespace util  {
/* ************************************************************************** *
 * FifoLink implementation
 * ************************************************************************** */

//Inserts this node to end of FIFO queue.
void FifoLink::push(FifoLink * & fifo_head, FifoLink * & fifo_tail)
{
  if (fifo_tail)
    fifo_tail->_prev = this;
  _next = fifo_tail;
  fifo_tail = this;
  if (!fifo_head)
    fifo_head = this;
  _prev = 0;
}
//Excludes this node from start of FIFO queue.
void FifoLink::pop(FifoLink * & fifo_head, FifoLink * & fifo_tail)
{
  fifo_head = _prev;
  _next = _prev = 0;
  if (!fifo_head)
    fifo_tail = 0;
  else
    fifo_head->_next = 0;
}
//excludes this node from queue
void FifoLink::unlink(FifoLink * & fifo_head, FifoLink * & fifo_tail)
{
  if (!_next) { //front element
    pop(fifo_head, fifo_tail);
    return;
  }
  if (!_prev) { //back element
    fifo_tail = _next;
    _next = 0;
    if (!fifo_tail)
      fifo_head = 0;
    return;
  }
  //intermediate element
  _next->_prev = _prev;
  _prev->_next = _next;
  _next = _prev = 0;
}

/* ************************************************************************** *
 * FifoQueue implementation
 * ************************************************************************** */
FifoLink * FifoList::pop_front(void)
{
  FifoLink * pNode = _head;
  if (_head)
    pNode->pop(_head, _tail);
  return pNode;
}

void FifoList::push_back(FifoLink * use_node)
{
  if (_tail)
    use_node->push(_head, _tail);
  _tail = use_node;
  if (!_head)
    _head = use_node;
}

} //util
} //smsc

