#ifndef __SMSC_CORE_BUFFERS_PODHASHMULTI_HPP__
#define __SMSC_CORE_BUFFERS_PODHASHMULTI_HPP__

#include "util/int.h"
#include <memory.h>
#include <vector>

namespace smsc{
namespace core{
namespace buffers{

#define PHNODEREF(r) nodePools[r.pidx].nodes[r.sidx]
#define PHVALUEREF(r) valuePools[r.pidx].nodes[r.sidx]


template <class K,class V,class HF>
class PODHashMulti{
public:
  explicit PODHashMulti(size_t n=0)
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
    valuePools=0;
    valuePoolsCount=0;
    freeNode=badReference;
    freeValue=badReference;
    itemsCount=0;
  }

  PODHashMulti(const PODHashMulti& rhs)
  {
    bucketsCount=0;
    nodePoolsCount=0;
    valuePoolsCount=0;
    freeNode=badReference;
    freeValue=badReference;
    *this=rhs;
  }

  ~PODHashMulti()
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
        AddValue(buckets[idx],v);
        buckets[idx].r.ccnt=0;
        bucketsUsed++;
        itemsCount++;
        return true;
      }else if(buckets[idx].r.ccnt==0)
      {
        if(buckets[idx].k==k)
        {
          AddValue(buckets[idx],v);
          return false;
        }
        NodeReference r=GetFreeNode();
        Node& n=PHNODEREF(r);
        n.k=k;
        n.v=badReference;
        AddValue(n,v);
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
          AddValue(n0,v);
          return false;
        }

        NodeReference r=FindNode(n0.r,k);
        if(r!=badReference)
        {
          AddValue(PHNODEREF(r),v);
          return false;
        }

        r=GetFreeNode();
        Node& n=PHNODEREF(r);
        n.k=k;
        n.v=badReference;
        AddValue(n,v);
        n.r=n0.r;
        r.ccnt=n0.r.ccnt;
        n0.r=r;
        n0.r.ccnt++;
        itemsCount++;
        return true;
      }
    }
  }

  bool Lookup(K k,std::vector<V>& v)
  {
    NodeReference vr;
    size_t idx;
    NodeReference nr;
    if(Lookup(k,vr,idx,nr))
    {
      while(vr!=badReference)
      {
        VNode& n=PHVALUEREF(vr);
        v.push_back(n.v);
        vr=n.nv;
      }
      return true;
    }
    return false;
  }
  
  bool Lookup(K k,std::vector<V*>& v)
  {
    NodeReference vr;
    size_t idx;
    NodeReference nr;
    if(Lookup(k,vr,idx,nr))
    {
      while(vr!=badReference)
      {
        VNode& n=PHVALUEREF(vr);
        v.push_back(&n.v);
        vr=n.nv;
      }
      return true;
    }
    return false;
  }
  
  bool DeleteValue(K k,const V& v)
  {
    NodeReference vr;
    size_t idx;
    NodeReference nr=badReference;
    if(Lookup(k,vr,idx,nr))
    {
      NodeReference pvr=vr;
      while(vr!=badReference)
      {
        VNode& n=PHVALUEREF(vr);
        if(n.v==v)
        {
          if(vr==pvr)
          {
            if(nr==badReference)
            {
              buckets[idx].v=n.nv;
            }else
            {
              PHNODEREF(nr).v=n.nv;
            }
          }else
          {
            PHVALUEREF(pvr).nv=n.nv;
          }
          FreeValue(vr);
          break;
        }
        pvr=vr;
        vr=n.nv;
      }
    }
    return false;
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
        FreeValueList(buckets[idx].v);
        buckets[idx].v=badReference;
        buckets[idx].r=badReference;
        bucketsUsed--;
        itemsCount--;
        return true;
      }
      return false;
    }
    if(buckets[idx].k==k)
    {
      FreeValueList(buckets[idx].v);
      buckets[idx].v=badReference;
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
        FreeValueList(n.v);
        n.v=badReference;
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

  PODHashMulti& operator=(const PODHashMulti& rhs)
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
    valuePoolsCount=rhs.valuePoolsCount;
    if(valuePoolsCount)
    {
      valuePools=new ValuePool[valuePoolsCount];
      for(uint32_t i=0;i<valuePoolsCount;i++)
      {
        valuePools[i].nodes=new VNode[rhs.valuePools[i].size];
        memcpy(valuePools[i].nodes,rhs.valuePools[i].nodes,sizeof(VNode)*rhs.valuePools[i].size);
      }
    }
    freeNode=rhs.freeNode;
    freeValue=rhs.freeValue;
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
    for(uint32_t i=0;i<valuePoolsCount;i++)
    {
      delete [] valuePools[i].nodes;
    }
    if(valuePoolsCount)delete [] valuePools;
    nodePools=0;
    nodePoolsCount=0;
    valuePools=0;
    valuePoolsCount=0;
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
    Iterator():h(0),idx(0),r(badReference),vr(badReference),first(true)
    {
    }
    Iterator(const PODHashMulti& argH):h(&argH),idx(0),r(badReference),vr(badReference),first(true)
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
          if(r==badReference && vr==badReference)
          {
            idx++;
            if(idx>=h->bucketsCount)return false;
          }
        }
        if(vr!=badReference)
        {
          if(r==badReference)
          {
            k=h->buckets[idx].k;
            VNode& vn=h->valuePools[vr.pidx].nodes[vr.sidx];
            v=vn.v;
            vr=vn.nv;
          }else
          {
            k=h->nodePools[r.pidx].nodes[r.sidx].k;
            VNode& vn=h->valuePools[vr.pidx].nodes[vr.sidx];
            v=vn.v;
            vr=vn.nv;
          }
          return true;
        }
        if(r!=badReference)
        {
          Node& n=h->nodePools[r.pidx].nodes[r.sidx];
          k=n.k;
          r=h->nodePools[r.pidx].nodes[r.sidx].r;
          vr=n.v;
          if(vr==badReference)
          {
            continue;
          }
          VNode& vn=h->valuePools[vr.pidx].nodes[vr.sidx];
          v=vn.v;
          vr=vn.nv;
          return true;
        }
        if(h->buckets[idx].r==badReference)continue;
        if(h->buckets[idx].r.ccnt!=0)
        {
          r=h->buckets[idx].r;
        }
        k=h->buckets[idx].k;
        vr=h->buckets[idx].v;
        if(vr==badReference)
        {
          continue;
        }
        VNode& vn=h->valuePools[vr.pidx].nodes[vr.sidx];
        v=vn.v;
        vr=vn.nv;
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
    const PODHashMulti* h;
    NodeReference r;
    NodeReference vr;
    size_t idx;
    bool first;
  };

  Iterator getIterator()
  {
    return Iterator(*this);
  }

protected:

  struct VNode{
    V v;
    NodeReference nv;
  };

  struct Node{
    K k;
    NodeReference v;
    NodeReference r;
  };

  bool Lookup(K k,NodeReference& vr,size_t& idx,NodeReference& vnr)
  {
    if(bucketsCount==0)return false;
    idx=HF::CalcHash(k)%bucketsCount;
    if(buckets[idx].r==badReference)return false;
    if(buckets[idx].k==k)
    {
      vr=buckets[idx].v;
      return true;
    }
    if(buckets[idx].r.ccnt==0)return false;
    NodeReference r=buckets[idx].r;
    while(r!=badReference)
    {
      Node &n=PHNODEREF(r);
      if(n.k==k)
      {
        vr=n.v;
        vnr=r;
        return true;
      }
      r=n.r;
    }
    return false;
  }


  void AddValue(Node& n,const V& v)
  {
    NodeReference vr=GetFreeValue();
    VNode& vn=PHVALUEREF(vr);
    vn.v=v;
    vn.nv=n.v;
    n.v=vr;
  }

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

  void Insert(K k,NodeReference v)
  {
      size_t idx=HF::CalcHash(k)%bucketsCount;
      if(buckets[idx].r==badReference)
      {
        buckets[idx].k=k;
        buckets[idx].v=v;
        buckets[idx].r.ccnt=0;
        bucketsUsed++;
        itemsCount++;
        return;
      }else if(buckets[idx].r.ccnt==0)
      {
        NodeReference r=GetFreeNode();
        Node& n=PHNODEREF(r);
        n.k=k;
        n.v=v;
        n.r=badReference;
        buckets[idx].r=r;
        buckets[idx].r.ccnt=1;
        itemsCount++;
        return;
      }else
      {
        Node& n0=buckets[idx];
        NodeReference r=FindNode(n0.r,k);
        if(r!=badReference)
        {
          PHNODEREF(r).v=v;
          return;
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
        return;
      }
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
        n.v=badReference;
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
  
  NodeReference GetFreeValue()
  {
    if(freeValue!=badReference)
    {
      NodeReference rv=freeValue;
      freeValue=PHVALUEREF(rv).nv;
      PHVALUEREF(rv).nv=badReference;
      return rv;
    }
    if(valuePoolsCount==0)
    {
      valuePools=new ValuePool[1];
      valuePoolsCount=1;
      uint32_t sz=bucketsCount;
      valuePools[0].nodes=new VNode[sz];
      valuePools[0].size=sz;
      valuePools[0].count=0;
    }
    if(valuePools[valuePoolsCount-1].count==valuePools[valuePoolsCount-1].size)
    {
      ValuePool* vp=new ValuePool[valuePoolsCount+1];
      memcpy(vp,valuePools,sizeof(ValuePool)*valuePoolsCount);
      delete [] valuePools;
      valuePools=vp;
      uint32_t sz=bucketsCount;
      valuePools[valuePoolsCount].nodes=new VNode[sz];
      valuePools[valuePoolsCount].size=sz;
      valuePools[valuePoolsCount].count=0;
      valuePoolsCount++;
    }
    NodeReference rv={
      valuePoolsCount-1,
      valuePools[valuePoolsCount-1].count++,
      0
    };
    return rv;
  }
  
  void FreeValue(NodeReference r)
  {
    valuePools[r.pidx].nodes[r.sidx].nv=freeValue;
    freeValue=r;
  }
  
  void FreeValueList(NodeReference r)
  {
    if(r==badReference)
    {
      return;
    }
    NodeReference fr=r;
    while(PHVALUEREF(r).nv!=badReference)
    {
      r=PHVALUEREF(r).nv;
    }
    PHVALUEREF(r).nv=freeValue;
    freeValue=fr;
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
  
  struct ValuePool{
    uint32_t size;
    uint32_t count;
    VNode* nodes;
  };
  ValuePool* valuePools;
  uint32_t valuePoolsCount;
  NodeReference freeValue;
  
  static NodeReference badReference;
};

template <class K,class V,class HF>
typename PODHashMulti<K,V,HF>::NodeReference PODHashMulti<K,V,HF>::badReference={0xffff,0xffffff,0xff};

}
}
}

#undef PHNODEREF
#undef PHVALUEREF

#endif

