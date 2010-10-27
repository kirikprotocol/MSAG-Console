#ifndef __SMSC_CORE_BUFFERS_XTREE_HPP__
#define __SMSC_CORE_BUFFERS_XTREE_HPP__

#include <vector>
#include <string.h>
#include <stdio.h>
#include <typeinfo>

namespace smsc{
namespace core{
namespace buffers{


template <int PGSZ=65536>
struct PooledAllocator{
template <class X>
struct Allocator{
  enum{FREENODESPOOLSIZE=16384};
  typedef X T;
  struct PoolPage
  {
    T pool[PGSZ];
    int allocated;
    PoolPage* next;
    PoolPage()
    {
      memset(pool,0,sizeof(pool));
      allocated=0;
      next=0;
    }
  };
  struct FreeNode
  {
    T* mem;
    FreeNode* next;
  };
  struct FreeNodesPage{
    FreeNode pool[FREENODESPOOLSIZE];
    int allocated;
    FreeNodesPage* next;
  };

  struct Initializer
  {
    Initializer()
    {
      Allocator::lastPage=&firstPage;
      memset(Allocator::freeArrays,0,sizeof(Allocator::freeArrays));
      Allocator::freeNodesPoolLastPage=&Allocator::freeNodesPoolFirstPage;
      Allocator::freeNodesList=0;
    }
    ~Initializer()
    {
      //PrintStats();
    }
  };

  static PoolPage firstPage;
  static PoolPage* lastPage;
  static FreeNode* freeArrays[256];
  static FreeNodesPage freeNodesPoolFirstPage;
  static FreeNodesPage* freeNodesPoolLastPage;
  static FreeNode* freeNodesList;
  static Initializer initer;

  static FreeNode* NewFreeNode()
  {
    if(freeNodesList)
    {
      FreeNode* rv=freeNodesList;
      freeNodesList=freeNodesList->next;
      return rv;
    }
    if(freeNodesPoolLastPage->allocated<FREENODESPOOLSIZE)
    {
      return freeNodesPoolLastPage->pool+freeNodesPoolLastPage->allocated++;
    }
    static Initializer* x=&initer;
    freeNodesPoolLastPage->next=new FreeNodesPage;
    freeNodesPoolLastPage=freeNodesPoolLastPage->next;
    return freeNodesPoolLastPage->pool+freeNodesPoolLastPage->allocated++;
  }

  static T* New()
  {
    if(freeArrays[1])
    {
      FreeNode* ptr=freeArrays[1];
      freeArrays[1]=freeArrays[1]->next;
      ptr->next=freeNodesList;
      memset(ptr->mem,0,sizeof(T));
      return ptr->mem;
    }
    if(lastPage->allocated<PGSZ)
    {
      return lastPage->pool+lastPage->allocated++;
    }
    lastPage->next=new PoolPage;
    lastPage=lastPage->next;
    return lastPage->pool+lastPage->allocated++;
  }
  static T* NewArray(int n)
  {
    if(freeArrays[n])
    {
      FreeNode* ptr=freeArrays[n];
      freeArrays[n]=freeArrays[n]->next;
      ptr->next=freeNodesList;
      memset(ptr->mem,0,n*sizeof(T));
      return ptr->mem;
    }
    if(lastPage->allocated+n<=PGSZ)
    {
      T* rv=lastPage->pool+lastPage->allocated;
      lastPage->allocated+=n;
      return rv;
    }
    if(lastPage->allocated!=PGSZ)
    {
      FreeNode* ptr=NewFreeNode();
      ptr->mem=lastPage->pool+lastPage->allocated;
      ptr->next=freeArrays[PGSZ-lastPage->allocated];
      freeArrays[PGSZ-lastPage->allocated]=ptr;
    }
    lastPage->next=new PoolPage;
    lastPage=lastPage->next;
    T* rv=lastPage->pool;
    lastPage->allocated+=n;
    return rv;
  }
  static void Delete(T* mem)
  {
    FreeNode* ptr=NewFreeNode();
    ptr->mem=mem;
    ptr->next=freeArrays[1];
    freeArrays[1]=ptr;
  }
  static void DeleteArray(T* mem,int n)
  {
    FreeNode* ptr=NewFreeNode();
    ptr->mem=mem;
    ptr->next=freeArrays[n];
    freeArrays[n]=ptr;
  }
  static void PrintStats()
  {
    printf("type:%s\n",typeid(T).name());
    int cnt=0;
    PoolPage* ptr=&firstPage;
    while(ptr)
    {
      cnt++;ptr=ptr->next;
    }
    printf("Pool pages:%d(%d bytes)\n",cnt,cnt*PGSZ*sizeof(T));
    for(int i=0;i<256;i++)
    {
      cnt=0;
      FreeNode* fptr=freeArrays[i];
      while(fptr)
      {
        cnt++;fptr=fptr->next;
      }
      if(cnt)printf("%d:%d\n",i,cnt);
    }
  }
};
};

template <int PGSZ>
template <class T>
typename PooledAllocator<PGSZ>::template Allocator<T>::PoolPage PooledAllocator<PGSZ>::Allocator<T>::firstPage;
template <int PGSZ>
template <class T>
typename PooledAllocator<PGSZ>::template Allocator<T>::PoolPage* PooledAllocator<PGSZ>::Allocator<T>::lastPage;
template <int PGSZ>
template <class T>
typename PooledAllocator<PGSZ>::template Allocator<T>::FreeNode* PooledAllocator<PGSZ>::Allocator<T>::freeArrays[256];
template <int PGSZ>
template <class T>
typename PooledAllocator<PGSZ>::template Allocator<T>::FreeNodesPage PooledAllocator<PGSZ>::Allocator<T>::freeNodesPoolFirstPage;
template <int PGSZ>
template <class T>
typename PooledAllocator<PGSZ>::template Allocator<T>::FreeNodesPage* PooledAllocator<PGSZ>::Allocator<T>::freeNodesPoolLastPage;
template <int PGSZ>
template <class T>
typename PooledAllocator<PGSZ>::template Allocator<T>::FreeNode* PooledAllocator<PGSZ>::Allocator<T>::freeNodesList;
template <int PGSZ>
template <class T>
typename PooledAllocator<PGSZ>::template Allocator<T>::Initializer PooledAllocator<PGSZ>::Allocator<T>::initer;



struct HeapAllocator{
template <class T>
struct Allocator{
  static T* New()
  {
    return (T*)memset(new T,0,sizeof(T));
  }
  static T* NewArray(int n)
  {
    return (T*)memset(new T[n],0,n*sizeof(T));
  }
  static void Delete(T* ptr)
  {
    delete ptr;
  }
  static void DeleteArray(T* ptr,int n)
  {
    delete [] ptr;
  }
};
};

template <bool cond,class TRUECLASS,class FALSECLAS>
struct BinChoice
{
  typedef FALSECLAS Value;
};

template <class TRUECLASS,class FALSECLAS>
struct BinChoice<true,TRUECLASS,FALSECLAS>
{
  typedef TRUECLASS Value;
};


static unsigned char rndArr[256]={
153,178,253,86,174,28,140,133,60,126,42,105,109,29,182,154,63,145,238,242,22,236,43,55,113,80,197,112,48,142,152,20,44,239,88,69,75,50,10,111,150,144,243,138,121,156,97,76,139,11,72,190,65,0,201,172,155,12,125,119,188,94,250,37,30,192,173,90,59,163,147,31,255,56,117,19,194,17,82,47,217,15,4,237,116,148,175,179,129,195,206,71,211,244,137,3,229,70,108,241,157,245,84,184,167,207,74,16,227,180,189,177,23,77,85,101,136,5,221,118,62,164,240,26,246,248,114,46,158,25,33,2,252,212,168,183,166,91,176,78,38,224,64,1,45,54,14,249,67,87,106,228,216,39,205,149,123,9,49,52,79,36,57,165,209,115,191,230,103,215,235,254,232,21,200,193,6,41,213,98,73,202,8,27,143,223,199,196,68,81,141,146,247,35,151,171,204,95,34,185,99,219,122,40,53,181,170,93,169,18,124,128,83,226,110,32,187,234,24,222,107,220,127,225,218,198,251,96,203,233,66,104,214,210,231,51,13,160,135,132,134,208,120,159,102,58,61,89,161,162,100,92,186,130,7,131
};

template <class T,class MEMMAN=HeapAllocator,bool UseHashNode=false>
class XTree{
protected:

  friend struct HashNode;
  struct HashNode
  {
    char*  subChildren;
    HashNode** subNodes;
    HashNode*  maskNode;
    T* data;
    short int subCount;
    short int subSize;

    inline HashNode* Find(char c)const
    {
      if(subSize==0)return 0;
      int shift=0;
      unsigned idx;
      int mask=subSize-1;
      unsigned char tmp=c;
      while(shift<subCount)
      {
        idx=tmp&mask;
        if(subChildren[idx]==c)return subNodes[idx];
        if(!subChildren[idx])return 0;
        shift++;
        tmp=rndArr[tmp];
      }
      return 0;
    }
    HashNode* FindInsert(char c)
    {
      if(subSize==0)
      {
        const int initSize=4;
        int idx=c&(initSize-1);
        subChildren=MEMMAN::template Allocator<char>::NewArray(initSize);
        subNodes=MEMMAN::template Allocator<HashNode*>::NewArray(initSize);
        subNodes[idx]=MEMMAN::template Allocator<HashNode>::New();
        subChildren[idx]=c;
        subSize=initSize;
        subCount=1;
        return subNodes[idx];
      }
      int idx;
      int mask=subSize-1;
      int shift=0;
      unsigned char tmp=c;
      for(;;)
      {
        idx=tmp&mask;
        if(subChildren[idx]==c)return subNodes[idx];
        if(subChildren[idx]==0)
        {
          subChildren[idx]=c;
          subCount++;
          return subNodes[idx]=MEMMAN::template Allocator<HashNode>::New();
        }
        shift++;
        tmp=rndArr[tmp];
        if((shift>subCount || (subCount+subCount+subCount)>(subSize<<2)) && subSize<256)
        {
          int newsz=subSize*2;
          repeat:
          int newmask=newsz-1;
          int newidx,newshift=0;
          unsigned char newtmp;
          char* newChilndren=MEMMAN::template Allocator<char>::NewArray(newsz);
          HashNode** newNodes=MEMMAN::template Allocator<HashNode*>::NewArray(newsz);
          for(int i=0;i<subSize;i++)
          {
            if(subChildren[i]==0)continue;
            newtmp=subChildren[i];
            for(;;)
            {
              newidx=newtmp&newmask;
              if(newChilndren[newidx]==0)
              {
                newChilndren[newidx]=subChildren[i];
                newNodes[newidx]=subNodes[i];
                break;
              }
              newshift++;
              if(newshift>=subCount && newsz<1024)
              {
                MEMMAN::template Allocator<char>::DeleteArray(newChilndren,newsz);
                MEMMAN::template Allocator<HashNode*>::DeleteArray(newNodes,newsz);
                newsz<<=1;
                goto repeat;
              }
              newtmp=rndArr[newtmp];
            }
          }
          MEMMAN::template Allocator<char>::DeleteArray(subChildren,subSize);
          MEMMAN::template Allocator<HashNode*>::DeleteArray(subNodes,subSize);
          subChildren=newChilndren;
          subNodes=newNodes;
          subSize=newsz;
          mask=subSize-1;
          shift=0;
          tmp=c;
        }
      }
      return 0;
    }

    void Clear()
    {
      for(int i=0;i<subSize;i++)
      {
        if(subChildren[i])
        {
          subNodes[i]->Clear();
          MEMMAN::template Allocator<HashNode>::Delete(subNodes[i]);
        }
      }
      if(subSize)
      {
        MEMMAN::template Allocator<char>::DeleteArray(subChildren,subSize);
        MEMMAN::template Allocator<HashNode*>::DeleteArray(subNodes,subSize);
      }
      if(maskNode)
      {
        maskNode->Clear();
	MEMMAN::template Allocator<HashNode>::Delete(maskNode);
      }
      if(data)delete data;
      subSize=0;
    }
  };

  friend struct TreeNode;
  struct TreeNode
  {
    char*  subChildren;
    TreeNode** subNodes;
    TreeNode*  maskNode;
    T* data;
    unsigned short subCount;
    unsigned short subSize;
    TreeNode* Find(char c)const
    {
      int idx=BinFind(c)-1;
      if(idx>=0 && idx<subCount && subChildren[idx]==c)return subNodes[idx];
      return 0;
    }
    TreeNode* FindInsert(char c)
    {
      if(subCount==0)
      {
        subChildren=MEMMAN::template Allocator<char>::NewArray(2);
        *subChildren=c;
        subNodes=MEMMAN::template Allocator<TreeNode*>::NewArray(2);
        subNodes[0]=MEMMAN::template Allocator<TreeNode>::New();
        subCount=1;
        subSize=2;
        return subNodes[0];
      }
      int idx=BinFind(c);
      if(idx>0 && idx<=subCount && subChildren[idx-1]==c)
      {
        return subNodes[idx-1];
      }
      if(subCount==subSize)
      {
        char* newSC=MEMMAN::template Allocator<char>::NewArray(subSize*2);
        TreeNode** newSN=MEMMAN::template Allocator<TreeNode*>::NewArray(subSize*2);
        memcpy(newSC,subChildren,idx);
        memcpy(newSN,subNodes,idx*sizeof(TreeNode*));
        memcpy(newSC+idx+1,subChildren+idx,subCount-idx);
        memcpy(newSN+idx+1,subNodes+idx,(subCount-idx)*sizeof(TreeNode*));
        MEMMAN::template Allocator<char>::DeleteArray(subChildren,subSize);
        MEMMAN::template Allocator<TreeNode*>::DeleteArray(subNodes,subSize);
        subSize*=2;
        subChildren=newSC;
        subNodes=newSN;
      }else
      {
        memmove(subChildren+idx+1,subChildren+idx,subCount-idx);
        memmove(subNodes+idx+1,subNodes+idx,(subCount-idx)*sizeof(TreeNode*));
      }
      subChildren[idx]=c;
      subNodes[idx]=MEMMAN::template Allocator<TreeNode>::New();
      subCount++;
      return subNodes[idx];
    }

    int BinFind(char c)const
    {
      int cnt=subCount;
      int cnt2;
      int off=0;
      while(cnt>0)
      {
        cnt2=cnt/2;
        if(!(c<subChildren[off+cnt2]))
        {
          off+=cnt2+1;
          cnt-=cnt2+1;
        }else
        {
          if(subChildren[off+cnt2]==c)return off+cnt2;
          cnt=cnt2;
        }
      }
      return off;
    }
    void Clear()
    {
      for(int i=0;i<subCount;i++)
      {
        subNodes[i]->Clear();
        MEMMAN::template Allocator<TreeNode>::Delete(subNodes[i]);
      }
      if(subCount)
      {
        MEMMAN::template Allocator<char>::DeleteArray(subChildren,subSize);
        MEMMAN::template Allocator<TreeNode*>::DeleteArray(subNodes,subSize);
      }
      if(data)delete data;
      if(maskNode)
      {
        maskNode->Clear();
	MEMMAN::template Allocator<TreeNode>::Delete(maskNode);
      }
      subCount=0;
    }
    void Delete(char k)
    {
      int idx=BinFind(k)-1;
      if(subNodes[idx]->data)
      {
        delete subNodes[idx]->data;
        subNodes[idx]->data=0;
      }
      MEMMAN::template Allocator<TreeNode>::Delete(subNodes[idx]);
      memcpy(subChildren+idx,subChildren+idx+1,subCount-idx-1);
      memcpy(subNodes+idx,subNodes+idx+1,subCount-idx-1);
      subCount--;
    }
  };
  typedef typename BinChoice<UseHashNode,HashNode,TreeNode>::Value Node;
public:
  XTree()
  {
    root=new Node;
    memset(root,0,sizeof(Node));
    count=0;
  }

  ~XTree()
  {
    Clear();
    delete root;
  }
  T& Insert(const char* key,const T& data)
  {
    Node* ptr=root;
    while(*key)
    {
      if(*key=='?')
      {
        if(!ptr->maskNode)ptr->maskNode=MEMMAN::template Allocator<Node>::New();
        ptr=ptr->maskNode;
      }else
      {
        ptr=ptr->FindInsert(*key);
      }
      key++;
    }
    if(!ptr->data)
    {
      ptr->data=new T(data);
      count++;
    }
    else
    {
      *(ptr->data)=data;
    }
    return *(ptr->data);
  }
  bool Find(const char* key,T& data)const
  {
    if(!root)return false;
    if(FindRec(root,key,data))return true;
    Node* ptr=root->maskNode;
    key++;
    while(ptr && *key)
    {
      ptr=ptr->maskNode;
      key++;
    }
    if(ptr && ptr->data)
    {
      data=*ptr->data;
      return true;
    }
    return false;
  }

  void Clear()
  {
    root->Clear();
  }
  void Swap(XTree& that)
  {
    Node* n=root;
    root=that.root;
    that.root=n;
    int cnt=that.count;
    that.count=count;
    count=cnt;
  }

  T* Extract(const char* key)
  {
    Node* ptr=root;
    while(ptr)
    {
      if(!*key)
      {
        T* rv=ptr->data;
        ptr->data=0;
        return rv;
      }
      if(*key=='?')
      {
        ptr=ptr->maskNode;
      }
      else
      {
        ptr=ptr->Find(*key);
      }
      key++;
    }
    return 0;
  }

  void Delete(const char* key)
  {
    T* data=Extract(key);
    if(data)delete data;
  }

  int Count()
  {
    return count;
  }

  template <class Functor>
  void ForEach(Functor& op)
  {
    ForEachRecursive(root,op);
  }

protected:

  XTree(const XTree&);
  void operator=(const XTree&);

  Node* root;
  int count;

  template <class Functor>
  void ForEachRecursive(Node* ptr,Functor& op)
  {
    if(ptr->data)op(*ptr->data);
    if(UseHashNode)
    {
      for(int i=0;i<ptr->subSize;i++)
      {
        if(ptr->subChildren[i])
        {
          ForEachRecursive(ptr->subNodes[i],op);
        }
      }
    }else
    {
      for(int i=0;i<ptr->subCount;i++)
      {
        ForEachRecursive(ptr->subNodes[i],op);
      }
    }
  }

  bool FindRec(Node* ptr,const char* key,T& data)const
  {
    ptr=ptr->Find(*key);
    key++;
    while(ptr)
    {
      if(!*key)
      {
        if(ptr->data)
        {
          data=*ptr->data;
          return true;
        }
        return false;
      }
      if(ptr->maskNode)
      {
        if(FindRec(ptr,key,data))return true;
        ptr=ptr->maskNode;
        key++;
        while(ptr && *key)
        {
          ptr=ptr->maskNode;
          key++;
        }
        if(ptr && ptr->data)
        {
          data=*ptr->data;
          return true;
        }
        return false;
      }
      ptr=ptr->Find(*key);
      key++;
    }
    return false;
  }

};


}//buffers
}//core
}//smsc

#endif
