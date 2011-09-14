#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "core/buffers/FifoList.hpp"

namespace smsc {
namespace core {
namespace buffers {
/* ************************************************************************** *
 * FifoLink implementation
 * ************************************************************************** */

//Inserts this node to end of FIFO queue.
void FifoLink::push(FifoLink * & fifo_head, FifoLink * & fifo_tail)
{
  if (fifo_tail)
    fifo_tail->mPrev = this;
  mNext = fifo_tail;
  fifo_tail = this;
  if (!fifo_head)
    fifo_head = this;
  mPrev = 0;
}
//Excludes this node from start of FIFO queue.
void FifoLink::pop(FifoLink * & fifo_head, FifoLink * & fifo_tail)
{
  fifo_head = mPrev;
  mNext = mPrev = 0;
  if (!fifo_head)
    fifo_tail = 0;
  else
    fifo_head->mNext = 0;
}
//excludes this node from queue
void FifoLink::unlink(FifoLink * & fifo_head, FifoLink * & fifo_tail)
{
  if (!mNext) { //front element
    pop(fifo_head, fifo_tail);
    return;
  }
  if (!mPrev) { //back element
    fifo_tail = mNext;
    mNext = 0;
    if (!fifo_tail)
      fifo_head = 0;
    return;
  }
  //intermediate element
  mNext->mPrev = mPrev;
  mPrev->mNext = mNext;
  mNext = mPrev = 0;
}

/* ************************************************************************** *
 * FifoList implementation
 * ************************************************************************** */
FifoLink * FifoList::pop_front(void)
{
  FifoLink * pNode = mHead;
  if (mHead)
    pNode->pop(mHead, mTail);
  return pNode;
}

void FifoList::push_back(FifoLink & use_node)
{
  if (mTail)
    use_node.push(mHead, mTail);
  mTail = &use_node;
  if (!mHead)
    mHead = &use_node;
}

} //buffers
} //core
} //smsc

