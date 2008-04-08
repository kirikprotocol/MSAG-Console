#ifndef __SUA_UTILX_PREFIXTREE_HPP__
# define __SUA_UTILX_PREFIXTREE_HPP__

# include <core/buffers/XTree.hpp>

namespace utilx {

template <class T,class MEMMAN=smsc::core::buffers::HeapAllocator<>,bool UseHashNode=false>
class PrefixTree : public smsc::core::buffers::XTree<T,MEMMAN,UseHashNode> {
public:
  bool FindPrefix(const char* key,T& data)const
  {
    Node* ptr=root;
    T* foundData=0;
    while(*key && ptr)
    {
      if(ptr->data)
      {
        foundData=ptr->data;
      }
      ptr=ptr->Find(*key++);
    }
    if(foundData)
    {
      data=*foundData;
      return true;
    }
    return false;
  }
};

}

#endif
