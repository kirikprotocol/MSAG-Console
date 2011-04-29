#ifndef __SMSC_CORE_BUFFERS_HASHTREE_HPP__
#define __SMSC_CORE_BUFFERS_HASHTREE_HPP__

#ifdef __SunOS_5_9
#include <inttypes.h>
#else
#include <stdint.h>
#endif
#include <memory.h>
#include <string>

namespace smsc{
namespace core{
namespace buffers{

struct HTDefaultHashFunc{
  uint32_t operator()(const std::string& a_str)
  {
    unsigned char* ptr=(unsigned char*)a_str.c_str();
    uint32_t rv=0x7654321**ptr;
    while(*ptr)
    {
      rv+=37*rv+*ptr;
      ptr++;
    }
    return rv;
  }
  uint32_t operator()(uint32_t a_val)
  {
    return a_val;
  }
  uint32_t operator()(uint64_t a_val)
  {
    return (uint32_t)a_val;
  }
};

template <class K,class V>
struct KeyVal{
   const K m_key;
   V m_value;
   KeyVal(){}
   KeyVal(const K& a_key,const V& a_value):m_key(a_key),m_value(a_value){}
   const K& key()const
   {
      return m_key;
   }
   V& value()
   {
      return m_value;
   }
   const V& value()const
   {
      return m_value;
   }
};


template <class K,class V,typename HF=HTDefaultHashFunc>
class HashTree{
protected:
   struct Node;
public:

   typedef KeyVal<K,V> value_type;

   explicit HashTree(const HF& a_hashFunc=HF()):m_root(0),m_count(0),m_hashFunc(a_hashFunc),m_begin(&m_end),m_pool(0)
   {
      m_end.m_dnode.m_prev=0;
      m_end.m_dnode.m_next=0;
   }

   HashTree(const HashTree& a_other):m_root(0),m_count(0),m_hashFunc(a_other.m_hashFunc),m_begin(&m_end)
   {
      for(const_iterator it=a_other.begin(),end=a_other.end();it!=end;++it)
      {
         insert(it->m_key,it->m_value);
      }
   }

   HashTree& operator=(const HashTree& a_other)
   {
      if(this==&a_other)
      {
         return *this;
      }
      clear();
      for(const_iterator it=a_other.begin(),end=a_other.end();it!=end;++it)
      {
         insert(it->m_key,it->m_value);
      }
      return *this;
   }

   bool operator==(const HashTree& a_other)const
   {
      if(this==&a_other)
      {
         return true;
      }
      const_iterator it1=begin(),end1=end(),it2=a_other.begin(),end2=a_other.end();
      for(;;++it1,++it2)
      {
         if(it1==end1)
         {
            if(it2==end2)
            {
               return true;
            }
            return false;
         }
         if(it2==end2)
         {
            return false;
         }
         if(!(it1->m_key==it2->m_key) || !(it1->m_value==it2->m_value))
         {
            return false;
         }
      }
   }
   bool operator!=(const HashTree& a_other)const
   {
      return !operator==(a_other);
   }

   ~HashTree()
   {
      clear(true);
   }

   class iterator{
   public:
      iterator():m_node(0){}
      iterator operator++(int)
      {
         Node* rv=m_node;
         m_node=m_node->m_dnode.m_next;
         return iterator(rv);
      }

      iterator operator++()
      {
         m_node=m_node->m_dnode.m_next;
         return *this;
      }

      iterator operator--(int)
      {
         Node* rv=m_node;
         m_node=m_node->m_dnode.m_prev;
         return iterator(rv);
      }

      iterator operator--()
      {
         m_node=m_node->m_dnode.m_prev;
         return *this;
      }

      iterator& operator=(const iterator& a_other)
      {
         m_node=a_other.m_node;
         return *this;
      }

      bool operator==(const iterator& a_other)const
      {
         return m_node==a_other.m_node;
      }

      bool operator!=(const iterator& a_other)const
      {
         return m_node!=a_other.m_node;
      }

      value_type& operator*()
      {
         return *m_node->m_dnode.m_data;
      }
      value_type* operator->()
      {
         return m_node->m_dnode.m_data;
      }
      const value_type& operator*()const
      {
         return *m_node->m_dnode.m_data;
      }
      const value_type* operator->()const
      {
         return m_node->m_dnode.m_data;
      }
   protected:
      iterator(Node* a_node):m_node(a_node){}
      friend class HashTree;
      typename HashTree::Node* m_node;
   };

   class const_iterator{
   public:
      const_iterator():m_node(0){}
      const_iterator operator++(int)
      {
         const Node* rv=m_node;
         m_node=m_node->m_dnode.m_next;
         return const_iterator(rv);
      }

      const_iterator operator++()
      {
         m_node=m_node->m_dnode.m_next;
         return *this;
      }

      const_iterator operator--(int)const
      {
         Node* rv=m_node;
         m_node=m_node->m_dnode.m_prev;
         return const_iterator(rv);
      }

      const_iterator operator--()
      {
         m_node=m_node->m_dnode.m_prev;
         return *this;
      }

      const_iterator& operator=(const const_iterator& a_other)
      {
         m_node=a_other.m_node;
         return *this;
      }

      bool operator==(const const_iterator& a_other)const
      {
         return m_node==a_other.m_node;
      }

      bool operator!=(const const_iterator& a_other)const
      {
         return m_node!=a_other.m_node;
      }

      const value_type& operator*()const
      {
         return *m_node->m_dnode.m_data;
      }
      const value_type* operator->()const
      {
         return m_node->m_dnode.m_data;
      }
   protected:
      const_iterator(const Node* a_node):m_node(a_node){}
      friend class HashTree;
      const typename HashTree::Node* m_node;
   };


   iterator begin()
   {
      return iterator(m_begin);
   }

   iterator end()
   {
      return iterator(&m_end);
   }

   const_iterator begin()const
   {
      return const_iterator(m_begin);
   }

   const_iterator end()const
   {
      return const_iterator(&m_end);
   }

   iterator insert(const K& a_key,const V& a_value)
   {
      uint32_t hashCode=m_hashFunc(a_key);
      size_t bitIndex=0;
      if(!m_root)
      {
         if(!m_pool)
         {
           m_pool=new MemPool;
         }
         m_root=m_pool->newNode();
         m_root->m_ch[3]=m_root->m_ch[2]=m_root->m_ch[1]=m_root->m_ch[0]=0;
      }
      Node* ptr=m_root;
      Node* parent=0;
      unsigned char parentIdx=0;
      unsigned char idx;
      for(;;)
      {
         idx=hashCode&3;
         if(ptr->m_dnode.m_nodeType==ntSingleValue)
         {
            if(ptr->m_dnode.m_data->m_key==a_key)
            {
               ptr->m_dnode.m_data->m_value=a_value;
               return iterator(ptr);
            }
            if(bitIndex!=30)
            {
               uint32_t oldHashCode=m_hashFunc(ptr->m_dnode.m_data->m_key);
               oldHashCode>>=bitIndex;
               do
               {
                  Node* newNode=parent->m_ch[parentIdx]=m_pool->newNode();
                  newNode->m_ch[3]=newNode->m_ch[2]=newNode->m_ch[1]=newNode->m_ch[0]=0;
                  if((oldHashCode&3)!=idx)
                  {
                     newNode->m_ch[oldHashCode&3]=ptr;
                     ptr=newNode;
                     Node& n=*(ptr=ptr->m_ch[idx]=m_pool->newNode());
                     n.m_dnode.m_data=new value_type(a_key,a_value);
                     n.m_dnode.m_next=&m_end;
                     n.m_dnode.m_prev=m_end.m_dnode.m_prev;
                     n.m_dnode.m_nodeType=ntSingleValue;
                     if(m_end.m_dnode.m_prev)
                     {
                        m_end.m_dnode.m_prev->m_dnode.m_next=ptr;
                     }else
                     {
                        m_begin=ptr;
                     }
                     m_end.m_dnode.m_prev=ptr;
                     m_count++;
                     return iterator(ptr);
                  }
                  parent=newNode;
                  parentIdx=idx;
                  hashCode>>=2;
                  oldHashCode>>=2;
                  bitIndex+=2;
                  idx=hashCode&3;
               }while(bitIndex<30);
            }

            Node* newNode=parent->m_ch[parentIdx]=m_pool->newNode();
            newNode->m_dnode.m_nodeType=ntValuesArray;
            ValuesArray& va=*(newNode->m_dnode.m_array=new ValuesArray);
            va.m_count=0;
            va.m_values=new Node*[2];
            va.m_values[0]=ptr;
            Node& n=*(va.m_values[1]=m_pool->newNode());
            n.m_dnode.m_data=new value_type(a_key,a_value);
            n.m_dnode.m_next=&m_end;
            n.m_dnode.m_prev=m_end.m_dnode.m_prev;
            if(m_end.m_dnode.m_prev)
            {
               m_end.m_dnode.m_prev->m_dnode.m_next=&n;
            }else
            {
               m_begin=&n;
            }
            m_end.m_dnode.m_prev=&n;
            m_count++;
            return iterator(&n);
         }
         if(ptr->m_dnode.m_nodeType==ntValuesArray)
         {
            ValuesArray& va=*ptr->m_dnode.m_array;
            for(size_t i=0;i<va.m_count;i++)
            {
               if(va.m_values[i]->m_dnode.m_data->m_key==a_key)
               {
                  va.m_values[i]->m_dnode.m_data->m_value=a_value;
                  return iterator(va.m_values[i]);
               }
            }
            Node** newArr=new Node*[va.m_count+1];
            memcpy(newArr,va.m_values,sizeof(Node*)*va.m_count);
            delete [] va.m_values;
            va.m_values=newArr;
            Node& n=*(ptr=m_pool->newNode());
            n.m_dnode.m_data=new value_type(a_key,a_value);
            n.m_dnode.m_next=&m_end;
            n.m_dnode.m_prev=m_end.m_dnode.m_prev;
            if(m_end.m_dnode.m_prev)
            {
               m_end.m_dnode.m_prev->m_dnode.m_next=ptr;
            }else
            {
               m_begin=ptr;
            }
            m_end.m_dnode.m_prev=ptr;
            va.m_values[va.m_count++]=ptr;
            m_count++;
            return iterator(ptr);
         }
         if(ptr->m_ch[idx])
         {
            parent=ptr;
            parentIdx=idx;
            ptr=ptr->m_ch[idx];
            hashCode>>=2;
            bitIndex+=2;
            continue;
         }else
         {
            Node& n=*(ptr=ptr->m_ch[idx]=m_pool->newNode());
            n.m_dnode.m_data=new value_type(a_key,a_value);
            n.m_dnode.m_next=&m_end;
            n.m_dnode.m_prev=m_end.m_dnode.m_prev;
            n.m_dnode.m_nodeType=ntSingleValue;
            if(m_end.m_dnode.m_prev)
            {
               m_end.m_dnode.m_prev->m_dnode.m_next=ptr;
            }else
            {
               m_begin=ptr;
            }
            m_end.m_dnode.m_prev=ptr;
            m_count++;
            return iterator(ptr);
         }
      }
      return end();
   }

   iterator insert(const value_type& a_keyval)
   {
      return insert(a_keyval.m_key,a_keyval.m_value);
   }

   iterator find(const K& a_key)
   {
      if(!m_root)
      {
         return end();
      }
      uint32_t hashCode=m_hashFunc(a_key);
      Node* ptr=m_root;
      while(ptr)
      {
         if(ptr->m_dnode.m_nodeType==ntSingleValue)
         {
            if(ptr->m_dnode.m_data->m_key==a_key)
            {
               return iterator(ptr);
            }
            return end();
         }
         if(ptr->m_dnode.m_nodeType==ntValuesArray)
         {
            ValuesArray& va=*ptr->m_dnode.m_array;
            for(size_t i=0;i<va.m_count;i++)
            {
               if(va.m_values[i]->m_dnode.m_data->m_key==a_key)
               {
                  return iterator(va.m_values[i]);
               }
            }
            return end();
         }
         ptr=ptr->m_ch[hashCode&3];
         hashCode>>=2;
      }
      return end();
   }

   const_iterator find(const K& a_key)const
   {
      if(!m_root)
      {
         return end();
      }
      uint32_t hashCode=m_hashFunc(a_key);
      Node* ptr=m_root;
      while(ptr)
      {
         if(ptr->m_dnode.m_nodeType==ntSingleValue)
         {
            if(ptr->m_dnode.m_data->m_key==a_key)
            {
               return const_iterator(ptr);
            }
            return end();
         }
         if(ptr->m_dnode.m_nodeType==ntValuesArray)
         {
            ValuesArray& va=*ptr->m_dnode.m_array;
            for(size_t i=0;i<va.m_count;i++)
            {
               if(va.m_values[i]->m_dnode.m_data->m_key==a_key)
               {
                  return const_iterator(va.m_values[i]);
               }
            }
            return end();
         }
         ptr=ptr->m_ch[hashCode&3];
         hashCode>>=2;
      }
      return end();
   }


   bool find(const K& a_key,V& a_valueRef)
   {
      iterator it=find(a_key);
      if(it!=end())
      {
         a_valueRef=it->m_value;
         return true;
      }
      return false;
   }

   void erase(const K& a_key)
   {
      if(!m_root)
      {
         return;
      }
      uint32_t hashCode=m_hashFunc(a_key);
      Node* ptr=m_root;
      Node* path[16];
      unsigned char pathIdx[16];
      size_t pathCount=0;
      while(ptr)
      {
         unsigned char idx=hashCode&3;
         if(ptr->m_dnode.m_nodeType!=ntSingleValue && ptr->m_dnode.m_nodeType!=ntValuesArray)
         {
            path[pathCount]=ptr;
            pathIdx[pathCount++]=idx;
            ptr=ptr->m_ch[idx];
            hashCode>>=2;
            continue;
         }
         if(ptr->m_dnode.m_nodeType==ntSingleValue)
         {
            if(!(ptr->m_dnode.m_data->m_key==a_key))
            {
               return;
            }
            m_count--;
         }else
         if(ptr->m_dnode.m_nodeType==ntValuesArray)
         {
            ValuesArray& va=*ptr->m_dnode.m_array;
            bool found=false;
            for(size_t i=0;i<va.m_count;i++)
            {
               if(va.m_values[i]->m_dnode.m_data->m_key==a_key)
               {
                  found=true;
                  Node* vptr=va.m_values[i];
                  if(vptr->m_dnode.m_prev)
                  {
                     vptr->m_dnode.m_prev->m_dnode.m_next=vptr->m_dnode.m_next;
                     vptr->m_dnode.m_next->m_dnode.m_prev=vptr->m_dnode.m_prev;
                  }else
                  {
                     m_begin=vptr->m_dnode.m_next;
                     m_begin->m_dnode.m_prev=0;
                  }
                  delete vptr->m_dnode.m_data;
                  m_pool->freeNode(vptr);
                  if(va.m_count>1)
                  {
                     Node** newValues=new Node*[va.m_count-1];
                     if(i>0)
                     {
                        memcpy(newValues,va.m_values,i*sizeof(Node*));
                     }
                     if(i<va.m_count-1)
                     {
                        memcpy(newValues+i,va.m_values+i+1,(va.m_count-i-1)*sizeof(Node*));
                     }
                     delete [] va.m_values;
                     va.m_values=newValues;
                     va.m_count--;
                     m_count--;
                     return;
                  }else
                  {
                     delete ptr->m_dnode.m_array;
                     m_pool->freeNode(ptr);
                     m_count--;
                     break;
                  }
               }
            }
            if(!found)
            {
               return;
            }
         }
         if(ptr->m_dnode.m_prev)
         {
            ptr->m_dnode.m_prev->m_dnode.m_next=ptr->m_dnode.m_next;
            ptr->m_dnode.m_next->m_dnode.m_prev=ptr->m_dnode.m_prev;
         }else
         {
            m_begin=ptr->m_dnode.m_next;
            m_begin->m_dnode.m_prev=0;
         }
         delete ptr->m_dnode.m_data;
         m_pool->freeNode(ptr);
         while(pathCount>0)
         {
            ptr=path[pathCount-1];
            idx=pathIdx[pathCount-1];
            ptr->m_ch[idx]=0;
            if(ptr->m_ch[0] || ptr->m_ch[1] || ptr->m_ch[2] || ptr->m_ch[3])
            {
               return;
            }
            m_pool->freeNode(ptr);
            if(ptr==m_root)
            {
               m_root=0;
            }
            pathCount--;
         }
         return;
      }
   }

   void erase(iterator a_it)
   {
      erase(a_it.m_node->m_dnode.m_data->m_key);
   }

   void erase(const_iterator a_it)
   {
      erase(a_it.m_node->m_dnode.m_data->m_key);
   }

   size_t size()const
   {
      return m_count;
   }

   bool empty()
   {
      return !m_count;
   }

   void clear(bool a_freeMemPool=false)
   {
      if(m_root)
      {
         recClear(m_root);
      }
      m_root=0;
      m_count=0;
      m_begin=&m_end;
      m_end.m_dnode.m_prev=0;
      if(a_freeMemPool && m_pool)
      {
        delete m_pool;
        m_pool=0;
      }
   }

   void swap(HashTree& a_other)
   {
      Node* tmpRoot=m_root;
      m_root=a_other.m_root;
      a_other.m_root=tmpRoot;
      size_t tmpCount=m_count;
      m_count=a_other.m_count;
      a_other.m_count=tmpCount;
      Node* tmpBegin=m_begin;
      m_begin=a_other.m_begin;
      a_other.m_begin=tmpBegin;
      MemPool* tmpPool=m_pool;
      m_pool=a_other.m_pool;
      a_other.m_pool=tmpPool;
      if(m_end.m_dnode.m_prev)
      {
         m_end.m_dnode.m_prev->m_dnode.m_next=&a_other.m_end;
      }
      if(a_other.m_end.m_dnode.m_prev)
      {
        a_other.m_end.m_dnode.m_prev->m_dnode.m_next=&m_end;
      }
      Node* tmpEnd=m_end.m_dnode.m_prev;
      m_end.m_dnode.m_prev=a_other.m_end.m_dnode.m_prev;
      a_other.m_end.m_dnode.m_prev=tmpEnd;
   }

protected:
   friend class iterator;
   friend class const_iterator;

   static const intptr_t ntSingleValue=intptr_t(-1);//0xffffffff,
   static const intptr_t ntValuesArray=intptr_t(-2);//0xfffffffe

   struct Node;

   struct ValuesArray{
      Node** m_values;
      size_t m_count;
   };

   struct Node{
      struct DataNode{
         union{
            value_type* m_data;
            ValuesArray* m_array;
         };
         Node* m_prev;
         Node* m_next;
         intptr_t m_nodeType;
      };
      union{
         DataNode m_dnode;
         Node* m_ch[4];
      };
   };

   Node* m_root;
   size_t m_count;
   HF m_hashFunc;
   Node m_end;
   Node* m_begin;

   enum{POOL_PAGE_SIZE=256};
   struct MemPool{
      struct PoolPage{
         PoolPage():m_current(m_nodes),m_nextPage(0){}
         Node m_nodes[POOL_PAGE_SIZE];
         Node* m_current;
         PoolPage* m_nextPage;
      };
      PoolPage m_firstPage;
      PoolPage* m_currentPage;
      Node* m_freeNodes;
      MemPool():m_currentPage(&m_firstPage),m_freeNodes(0){}
      ~MemPool()
      {
         PoolPage* ptr=m_firstPage.m_nextPage;
         PoolPage* optr;
         while(ptr)
         {
            optr=ptr;
            ptr=ptr->m_nextPage;
            delete optr;
         }
      }
      Node* newNode()
      {
         if(m_freeNodes)
         {
            Node* rv=m_freeNodes;
            m_freeNodes=m_freeNodes->m_dnode.m_next;
            return rv;
         }
         if(m_currentPage->m_current==m_currentPage->m_nodes+POOL_PAGE_SIZE)
         {
            m_currentPage=m_currentPage->m_nextPage=new PoolPage;
         }
         return m_currentPage->m_current++;
      }
      void freeNode(Node* a_ptr)
      {
         a_ptr->m_dnode.m_next=m_freeNodes;
         m_freeNodes=a_ptr;
      }
   };
   MemPool* m_pool;

   void recClear(Node* a_ptr)
   {
      if(a_ptr->m_dnode.m_nodeType==ntSingleValue)
      {
         delete a_ptr->m_dnode.m_data;
         m_pool->freeNode(a_ptr);
         return;
      }else if(a_ptr->m_dnode.m_nodeType==ntValuesArray)
      {
         ValuesArray& va=*(a_ptr->m_dnode.m_array);
         for(size_t i=0;i<va.m_count;i++)
         {
            delete va.m_values[i]->m_dnode.m_data;
            m_pool->freeNode(va.m_values[i]);
         }
         delete [] va.m_values;
         delete a_ptr->m_dnode.m_array;
         m_pool->freeNode(a_ptr);
         return;
      }
      if(a_ptr->m_ch[0])recClear(a_ptr->m_ch[0]);
      if(a_ptr->m_ch[1])recClear(a_ptr->m_ch[1]);
      if(a_ptr->m_ch[2])recClear(a_ptr->m_ch[2]);
      if(a_ptr->m_ch[3])recClear(a_ptr->m_ch[3]);
   }
};

template <class K,class V,typename HF>
const intptr_t HashTree<K,V,HF>::ntSingleValue;
template <class K,class V,typename HF>
const intptr_t HashTree<K,V,HF>::ntValuesArray;


}
}
}


#endif
