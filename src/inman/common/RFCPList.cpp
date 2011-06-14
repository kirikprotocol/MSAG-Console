#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/common/RFCPList.hpp"

namespace smsc   {
namespace util  {

/* ************************************************************************** *
 * class RFCPList
 * ************************************************************************** */
void RFCPList::cleanAll(void)
{
  iterator it = base_type::begin();
  while (it != end()) {
    iterator cit = it++;
    if (cit->isFreed() && !cit->isMarked())
      erase(cit);
  }
}

void RFCPList::releaseNode(RFCPList::iterator & it)
{
  if (it != end()) {
    if (!it->isMarked())
      erase(it);
    else
      it->release();
  }
}

RFCPList::iterator RFCPList::begin(void)
{
  iterator it = base_type::begin();
  if ((it != end()) && it->isFreed()) //skip released nodes
    nextNode(it);
  return it;
}

RFCPList::const_iterator RFCPList::begin(void) const
{
  const_iterator it = base_type::begin();
  if ((it != end()) && it->isFreed()) //skip released nodes
    nextNode(it);
  return it;
}


RFCPList::iterator  RFCPList::findNode(void * p_obj)
{
  iterator it = base_type::begin();
  while (it != end()) {
    iterator cit = it++;
    //check that current node may be erased!
    if (cit->isFreed()) {
      if (!cit->isMarked())
        erase(cit);
    } else if (cit->_value == p_obj)
      return cit;
  }
  return it;
}

RFCPList::const_iterator  RFCPList::findNode(void * p_obj) const
{
  const_iterator it = base_type::begin();
  while (it != end()) {
    if (!it->isFreed() && (it->_value == p_obj))
      break;
    ++it;
  }
  return it;
}

void RFCPList::nextNode(RFCPList::iterator & it)
{
  if (it != end()) {
    do {
      iterator cit = it++;
      //check that current node may be erased!
      if (cit->isFreed() && !cit->isMarked())
        erase(cit);
    } while ((it != end()) && it->isFreed()); //skip released nodes
  }
  return;
}

void RFCPList::nextNode(RFCPList::const_iterator & it) const
{
  if (it != end()) {
    while ((++it != end()) && it->isFreed()) //skip released nodes
      ;
  }
  return;
}

//
void RFCPList::prevNode(RFCPList::iterator & it)
{
  if (it != end()) {
    do {
      iterator cit = it--;
      //check that current node may be erased!
      if (cit->isFreed() && !cit->isMarked())
        erase(cit);
    } while ((it != end()) && it->isFreed()); //skip released nodes
  }
  return;
}

//
void RFCPList::prevNode(RFCPList::const_iterator & it) const
{
  if (it != end()) {
    while ((--it != end()) && it->isFreed()) //skip released nodes
      ;
  }
  return;
}


} //util
} //smsc

