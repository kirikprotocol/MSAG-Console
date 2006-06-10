#ifndef __SMSC_CORE_BUFFERS_DIGITALTREE_HPP__
#define __SMSC_CORE_BUFFERS_DIGITALTREE_HPP__

#include <stdexcept>

namespace smsc{
namespace core{
namespace buffers{

template <class T>
class DigitalTree{
public:
  DigitalTree()
  {
    root=new Node;
  }
  ~DigitalTree()
  {
    Clear();
    delete root;
  }
  void Insert(const char* key,const T& data)
  {
    Node* ptr=root;
    while(*key)
    {
      if(*key>='0' && *key<='9')
      {
        if(!ptr->children[*key-'0'])ptr->children[*key-'0']=new Node;
        ptr=ptr->children[*key-'0'];
      }else if(*key=='?')
      {
        if(!ptr->maskChild)ptr->maskChild=new Node;
        ptr=ptr->maskChild;
      }else
      {
        throw std::runtime_error("Unsupported symbol in key");
      }
      key++;
    }
    if(!ptr->data)ptr->data=new T;
    *ptr->data=data;
  }
  bool Find(const char* key,T& data)const
  {
    return FindRecursive(root,key,data);
  }
  void Clear()
  {
    ClearRecursive(root);
  }
  void Swap(DigitalTree& that)
  {
    Node* n=root;
    root=that.root;
    that.root=n;
  }
protected:

  DigitalTree(const DigitalTree&);
  void operator=(const DigitalTree&);

  struct Node{
    Node* children[10];
    Node* maskChild;
    T* data;
    Node()
    {
      memset(children,0,sizeof(children));
      maskChild=0;
      data=0;
    }
  };
  Node* root;

  bool FindRecursive(Node* ptr,const char* key,T& data)const
  {
    if(!*key)
    {
      if(!ptr->data)return false;
      data=*ptr->data;
      return true;
    }
    if(*key>='0' && *key<='9')
    {
      if(ptr->children[*key-'0'])
      {
        if(FindRecursive(ptr->children[*key-'0'],key+1,data))return true;
      }
      if(ptr->maskChild)return FindRecursive(ptr->maskChild,key+1,data);
      return false;
    }else
    {
      throw std::runtime_error("Unsupported symbol in key");
    }
  }

  void ClearRecursive(Node* ptr)
  {
    for(int i=0;i<10;i++)
    {
      if(ptr->children[i])
      {
        ClearRecursive(ptr->children[i]);
        delete ptr->children[i];
        ptr->children[i]=0;
      }
    }
    if(ptr->maskChild)
    {
      ClearRecursive(ptr->maskChild);
      delete ptr->maskChild;
      ptr->maskChild=0;
    }
    if(ptr->data)
    {
      delete ptr->data;
      ptr->data=0;
    }
  }
};

}//buffers
}//core
}//smsc

#endif
