#ifndef __SMSC_CORE_BUFFERS_PODHASH_HPP__
#define __SMSC_CORE_BUFFERS_PODHASH_HPP__

#include "util/int.h"
#include <memory.h>

namespace smsc{
namespace core{
namespace buffers{

#define PHNODEREF(r) nodePools[r.pidx].nodes[r.sidx]


template <class K,class V,class HF>
class PODHash{
public:
  explicit PODHash(size_t n=0)
  {
    if(n)
    {
      buckets=new Node[n];
      memset(buckets,-1,sizeof(Node)*n);
      bucketsCount=n;
    }else
    {
      buckets=0;
      bucketsCount=0;
    }
    bucketsUsed=0;
    nodePools=0;
    nodePoolsCount=0;
    freeNode=badReference;
    itemsCount=0;
  }

  PODHash(const PODHash& rhs)
  {
    bucketsCount=0;
    *this=rhs;
  }

  ~PODHash()
  {
    Clear();
  }

  bool Insert(K k,const V& v)
  {
    if(bucketsUsed>=bucketsCount)
    {
      Rehash();
    }
    for(;;)
    {
      size_t idx=HF::CalcHash(k)%bucketsCount;
      if(buckets[idx].r==badReference)
      {
        buckets[idx].k=k;
        buckets[idx].v=v;
        buckets[idx].r.ccnt=0;
        bucketsUsed++;
        itemsCount++;
        return true;
      }else if(buckets[idx].r.ccnt==0)
      {
        if(buckets[idx].k==k)
        {
          buckets[idx].v=v;
          return false;
        }
        NodeReference r=GetFreeNode();
        Node& n=PHNODEREF(r);
        n.k=k;
        n.v=v;
        n.r=badReference;
        buckets[idx].r=r;
        buckets[idx].r.ccnt=1;
        itemsCount++;
        return true;
      }else
      {
        Node& n0=buckets[idx];
        if(((n0.r.ccnt>=8 && bucketsUsed*2>bucketsCount) || n0.r.ccnt>=32) && n0.k!=k)
        {
          Rehash();
          continue;
        }
        if(n0.k==k)
        {
          n0.v=v;
          return false;
        }

        NodeReference r=FindNode(n0.r,k);
        if(r!=badReference)
        {
          PHNODEREF(r).v=v;
          return false;
        }

        r=GetFreeNode();
        Node& n=PHNODEREF(r);
        n.k=k;
        n.v=v;
        n.r=n0.r;
        r.ccnt=n0.r.ccnt;
        n0.r=r;
        n0.r.ccnt++;
        itemsCount++;
        return true;
      }
    }
  }

  bool Lookup(K k,V& v)
  {
    if(bucketsCount==0)return false;
    size_t idx=HF::CalcHash(k)%bucketsCount;
    if(buckets[idx].r==badReference)return false;
    if(buckets[idx].k==k)
    {
      v=buckets[idx].v;
      return true;
    }
    if(buckets[idx].r.ccnt==0)return false;
    NodeReference r=buckets[idx].r;
    while(r!=badReference)
    {
      Node &n=PHNODEREF(r);
      if(n.k==k)
      {
        v=n.v;
        return true;
      }
      r=n.r;
    }
    return false;
  }

  bool Get(K k,V& v)
  {
    return Lookup(k,v);
  }

  bool Delete(K k)
  {
    if(bucketsCount==0)return false;
    size_t idx=HF::CalcHash(k)%bucketsCount;
    if(buckets[idx].r==badReference)return false;
    if(buckets[idx].r.ccnt==0)
    {
      if(buckets[idx].k==k)
      {
        buckets[idx].r=badReference;
        bucketsUsed--;
        itemsCount--;
        return true;
      }
      return false;
    }
    if(buckets[idx].k==k)
    {
      NodeReference r=buckets[idx].r;
      Node& n=PHNODEREF(r);
      buckets[idx]=n;
      FreeNode(r);
      if(buckets[idx].r==badReference)
      {
        buckets[idx].r.ccnt=0;
      }
      itemsCount--;
      return true;
    }
    NodeReference r=buckets[idx].r;
    NodeReference pr=badReference;
    while(r!=badReference)
    {
      Node& n=PHNODEREF(r);
      if(n.k==k)
      {
        if(pr==badReference)
        {
          buckets[idx].r=n.r;
          if(n.r==badReference)buckets[idx].r.ccnt=0;
          FreeNode(r);
        }else
        {
          Node& np=PHNODEREF(pr);
          np.r=n.r;
          FreeNode(r);
        }
        itemsCount--;
        return true;
      }
      pr=r;
      r=n.r;
    }
    return false;
  }

  PODHash& operator=(const PODHash& rhs)
  {
    Clear();
    bucketsCount=rhs.bucketsCount;
    buckets=new Node[bucketsCount];
    memcpy(buckets,rhs.buckets,sizeof(Node)*bucketsCount);
    bucketsUsed=rhs.bucketsUsed;
    nodePoolsCount=rhs.nodePoolsCount;
    if(nodePoolsCount)
    {
      nodePools=new NodePool[nodePoolsCount];
      for(uint32_t i=0;i<nodePoolsCount;i++)
      {
        nodePools[i].nodes=new Node[rhs.nodePools[i].size];
        memcpy(nodePools[i].nodes,rhs.nodePools[i].nodes,sizeof(Node)*rhs.nodePools[i].size);
      }
    }
    freeNode=rhs.freeNode;
    itemsCount=rhs.itemsCount;
    return *this;
  }

  void Clear()
  {
    if(!bucketsCount)return;
    delete [] buckets;
    buckets=0;
    bucketsCount=0;
    bucketsUsed=0;
    for(uint32_t i=0;i<nodePoolsCount;i++)
    {
      delete [] nodePools[i].nodes;
    }
    if(nodePoolsCount)delete [] nodePools;
    nodePools=0;
    nodePoolsCount=0;
    itemsCount=0;
  }
  
  size_t getCount()
  {
    return itemsCount;
  }

  protected:
    struct NodeReference{
      uint16_t pidx;
      uint32_t sidx:24;
      uint32_t ccnt:8;
      bool operator==(const NodeReference& r)const
      {
        return pidx==r.pidx && sidx==r.sidx && ccnt==r.ccnt;
      }
      bool operator!=(const NodeReference& r)const
      {
        return pidx!=r.pidx || sidx!=r.sidx || ccnt!=r.ccnt;
      }
    };
  public:

  friend struct Iterator;
  struct Iterator{
    Iterator():h(0),idx(0),r(badReference),first(true)
    {
    }
    Iterator(const PODHash& argH):h(&argH),idx(0),r(badReference),first(true)
    {
    }


    bool Next(K& k,V& v)
    {
      if(h->bucketsCount==0)return false;
      for(;;)
      {
        if(first)
        {
          first=false;
        }else
        {
          if(r==badReference)
          {
            idx++;
            if(idx>=h->bucketsCount)return false;
          }
        }
        if(r!=badReference)
        {
          Node& n=h->nodePools[r.pidx].nodes[r.sidx];
          k=n.k;
          v=n.v;
          r=h->nodePools[r.pidx].nodes[r.sidx].r;
          return true;
        }
        if(h->buckets[idx].r==badReference)continue;
        if(h->buckets[idx].r.ccnt!=0)
        {
          r=h->buckets[idx].r;
        }
        k=h->buckets[idx].k;
        v=h->buckets[idx].v;
        return true;
      }
    }

    void First()
    {
      first=true;
      idx=0;
      r=badReference;
    }
  protected:
    const PODHash* h;
    NodeReference r;
    size_t idx;
    bool first;
  };

  Iterator getIterator()
  {
    return Iterator(*this);
  }

protected:


  struct Node{
    K k;
    V v;
    NodeReference r;
  };

  NodeReference FindNode(NodeReference r,K k)
  {
    while(r!=badReference)
    {
      Node& n=PHNODEREF(r);
      if(n.k==k)return r;
      r=n.r;
    }
    return r;
  }

  void Rehash()
  {
    if(bucketsCount==0)
    {
      bucketsCount=15;
      buckets=new Node[bucketsCount];
      memset(buckets,-1,sizeof(Node)*bucketsCount);
      bucketsUsed=0;
      return;
    }
    size_t newSize=(bucketsCount+1)*2-1;
    Node* oldBuckets=buckets;
    buckets=new Node[newSize];
    memset(buckets,-1,sizeof(Node)*newSize);
    size_t oldBucketsCount=bucketsCount;
    bucketsUsed=0;
    bucketsCount=newSize;
    itemsCount=0;
    for(size_t i=0;i<oldBucketsCount;i++)
    {
      if(oldBuckets[i].r==badReference)continue;
      Insert(oldBuckets[i].k,oldBuckets[i].v);
      if(oldBuckets[i].r.ccnt==0)continue;
      NodeReference r=oldBuckets[i].r;
      NodeReference r0;
      while(r!=badReference)
      {
        Node& n=PHNODEREF(r);
        Insert(n.k,n.v);
        r0=n.r;
        FreeNode(r);
        r=r0;
      }
    }
    delete [] oldBuckets;
  }

  NodeReference GetFreeNode()
  {
    if(freeNode!=badReference)
    {
      NodeReference rv=freeNode;
      freeNode=PHNODEREF(rv).r;
      return rv;
    }
    if(nodePoolsCount==0)
    {
      nodePools=new NodePool[1];
      nodePoolsCount=1;
      uint32_t sz=bucketsCount/4;
      nodePools[0].nodes=new Node[sz];
      nodePools[0].size=sz;
      nodePools[0].count=0;
    }
    if(nodePools[nodePoolsCount-1].count==nodePools[nodePoolsCount-1].size)
    {
      NodePool* np=new NodePool[nodePoolsCount+1];
      memcpy(np,nodePools,sizeof(NodePool)*nodePoolsCount);
      delete [] nodePools;
      nodePools=np;
      uint32_t sz=bucketsCount/4;
      nodePools[nodePoolsCount].nodes=new Node[sz];
      nodePools[nodePoolsCount].size=sz;
      nodePools[nodePoolsCount].count=0;
      nodePoolsCount++;
    }
    NodeReference rv={
      nodePoolsCount-1,
      nodePools[nodePoolsCount-1].count++,
      0
    };
    return rv;
  }

  void FreeNode(NodeReference r)
  {
    nodePools[r.pidx].nodes[r.sidx].r=freeNode;
    freeNode=r;
  }

  Node* buckets;
  size_t bucketsCount;
  size_t bucketsUsed;
  size_t itemsCount;
  struct NodePool{
    uint32_t size;
    uint32_t count;
    Node* nodes;
  };
  NodePool* nodePools;
  uint32_t nodePoolsCount;
  NodeReference freeNode;
  static NodeReference badReference;
};

template <class K,class V,class HF>
typename PODHash<K,V,HF>::NodeReference PODHash<K,V,HF>::badReference={0xffff,0xffffff,0xff};

}
}
}

#undef PHNODEREF

#endif

