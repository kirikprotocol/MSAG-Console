/* ************************************************************************** *
 * Reference Counting List of Pointers helper.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_REFCOUNTING_LIST_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_UTIL_REFCOUNTING_LIST_HPP

#include <list>

namespace smsc {
namespace util {

class RFCPNode { //Node of Reference Counting List of Pointers
private:
  mutable unsigned  _marked;  //counter of references to this node
  bool              _freed;   //node is to erase

public:
  void *  _value;

  explicit RFCPNode(void * p_obj = NULL)
    : _marked(0), _freed(false), _value(p_obj)
  { }
  ~RFCPNode()
  { }

  void release(void) { _freed = true; }

  void mark(void) const
  {
    if (!(++_marked))
      --_marked;
  }
  void unmark(void) const
  {
    if (_marked)
      --_marked;
  }

  bool isMarked(void) const { return _marked != 0; }
  bool isFreed(void) const { return _freed; }
};


class RFCPList : std::list<RFCPNode> {
protected:
  typedef std::list<RFCPNode>  base_type;

public:
  RFCPList()
  { }
  ~RFCPList()
  { }

  using std::list<RFCPNode>::iterator;
  using std::list<RFCPNode>::const_iterator;

  using std::list<RFCPNode>::end;
  using std::list<RFCPNode>::empty;
  using std::list<RFCPNode>::clear;

  //Returns 'it' of 1st alive node or end() if no node found.
  //Cleans up released nodes.
  iterator begin(void);
  //Returns 'it' of 1st alive node or end() if no node found.
  const_iterator begin(void) const;

  //
  iterator insert(iterator it, void * p_obj)
  {
    return std::list<RFCPNode>::insert(it, RFCPNode(p_obj));
  }
  //
  void push_back(void * p_obj)
  {
    std::list<RFCPNode>::insert(end(), RFCPNode(p_obj));
  }

  //Scans whole list for released nodes, and erases nodes
  //which may be safely deleted.
  void cleanAll(void);

  //Releases node (immediately deletes or marks this node as target for erasing)
  void releaseNode(iterator & it);

  void markNode(const iterator & it) const
  {
    if (it != end())
      it->mark();
  }
  //
  void unmarkNode(const iterator & it) const
  {
    if (it != end())
      it->unmark();
  }

  void markNode(const const_iterator & it) const
  {
    if (it != end())
      it->mark();
  }
  //
  void unmarkNode(const const_iterator & it) const
  {
    if (it != end())
      it->unmark();
  }

  //Searches list for a node containing given pointer.
  //Returns end() if no node found.
  //Cleans up released nodes.
  iterator findNode(void * p_obj);
  //Searches forward for 1st non-released node.
  //Sets 'it' to end() if no node found.
  //Cleans up released nodes.
  void nextNode(iterator & it);
  //Searches backward for 1st non-released node.
  //Sets 'it' to end() if no node found.
  //Cleans up released nodes.
  void prevNode(iterator & it);

  //Searches list for a node containing given pointer.
  //Returns end() if no node found.
  const_iterator findNode(void * p_obj) const;
  //Searches forward for 1st non-released node.
  //Sets 'it' to end() if no node found.
  void nextNode(const_iterator & it) const;
  //Searches backward for 1st non-released node.
  //Sets 'it' to end() if no node found.
  void prevNode(const_iterator & it) const;
};


} //util
} //smsc

#endif /* __SMSC_UTIL_REFCOUNTING_LIST_HPP */

