#ifndef __SMSC_CORE_BUFFERS_DISKXTREE_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_CORE_BUFFERS_DISKXTREE_HPP__

#include "core/buffers/File.hpp"
#include "util/int.h"
#include "util/Exception.hpp"
#include <vector>

namespace smsc{
namespace core{
namespace buffers{

class DuplicateKeyException:public std::exception{
public:
  DuplicateKeyException(const char* key)
  {
    error="Duplicate key:";
    error+=key;
  }
  ~DuplicateKeyException()throw()
  {

  }
  virtual const char * what() const throw()
  {
    return error.c_str();
  }
protected:
  std::string error;
};

template <class T>
class DiskXTree{
public:
  DiskXTree()
  {

  }
  ~DiskXTree()
  {
    Clear();
  }

  void Insert(const char* key,const T& value,bool unique=false)
  {
    Node* ptr=&root;
    const char* orgkey=key;
    while(*key)
    {
      if(ptr->chSize==0 && ptr->children==0 && ptr->child==0 && ptr->data==0)
      {
        ptr->nodeType=ntCollapsed;
        size_t l=strlen(key);
        if(l>7)
        {
          ptr->str=new char[l+1];
          memcpy(ptr->str,key,l+1);
          ptr->heapStr=true;
        }else
        {
          memcpy(ptr->Str(),key,l+1);
        }
        ptr->StoreData(value);
        return;
      }
      if(ptr->nodeType==ntCollapsed)
      {
        char* c=ptr->Str();
        while(*c && *c==*key)
        {
          c++;
          key++;
        }
        if(*c==0 && *key==0)
        {
          if(unique && ptr->data)
          {
            throw DuplicateKeyException(orgkey);
          }
          ptr->StoreData(value);
          return;
        }
        if(*key)
        {
          ptr=ptr->Split(c,ntNormal,key);
        }else
        {
          ptr=ptr->Split(c,ntCollapsed,key);
        }
        if(!*key)
        {
          if(unique && ptr->data)
          {
            throw DuplicateKeyException(orgkey);
          }
          ptr->StoreData(value);
          return;
        }
      }else
      {
        Node** pptr;
        ptr=ptr->FindInsert(*key,pptr);
        key++;
        if(!*key)
        {
          if(ptr->nodeType==ntCollapsed && ptr->Str() && *ptr->Str())
          {
            Node* n=new Node;
            *pptr=n;
            n->child=ptr;
            ptr=n;
          }
          if(unique && ptr->data)
          {
            throw DuplicateKeyException(orgkey);
          }
          ptr->StoreData(value);
          return;
        }
      }
    }
  }

  void Clear()
  {
    root.Clear();
  }


  void Dump()
  {
    File::offset_type off=6;
    PrepareOffsets(&root,off);
    Dump(&root,0,0);
  }

  void WriteToFile(File& f)
  {
    f.Write(getSig(),4);
    f.WriteNetInt16(getVer());
    File::offset_type off=6;
    PrepareOffsets(&root,off);
    root.Write(f);
  }

  void ReadFromFile(File& f)
  {
    char sig[4];
    f.Read(sig,4);
    if(memcmp(sig,getSig(),4)!=0)
    {
      throw smsc::util::Exception("Invalid signature of DiskXTree file");
    }
    uint16_t ver=f.ReadNetInt16();
    if(ver!=getVer())
    {
      throw smsc::util::Exception("Invalid version of DiskXTree file");
    }
    root.Clear();
    root.Read(f,true);
  }

  static void Lookup(File& f,const char* key,std::vector<T>& values)
  {
    Node node;
    f.Seek(6);
    node.Read(f,false);
    while(*key)
    {
      if(node.nodeType==ntCollapsed)
      {
        char* c=node.Str();
        while(*c && *c==*key)
        {
          c++;
          key++;
        }
        if(*c==0 && *key==0)
        {
          ReadData(f,node.dataOff,values);
          return;
        }
        if(*c)return;
        if(node.childOff)
        {
          f.Seek(node.childOff);
          node.Clear();
          node.Read(f,false);
          continue;
        }
        return;
      }else
      {
        uint8_t off=node.BinFind(*key)-1;
        if(off>=0 && off<node.chSize && node.Children()[off]==*key)
        {
          f.Seek(node.childsOff[off]);
          node.Clear();
          node.Read(f,false);
        }else
        {
          return;
        }
      }
      key++;
      if(!*key)
      {
        ReadData(f,node.dataOff,values);
        return;
      }
    }
  }

  void Lookup(const char* key,std::vector<T>& values)
  {
    Node* node=&root;
    while(*key)
    {
      if(node->nodeType==ntCollapsed)
      {
        char* c=node->Str();
        while(*c && *c==*key)
        {
          c++;
          key++;
        }
        if(*c==0 && *key==0)
        {
          CopyData(node,values);
          return;
        }
        if(*c)return;
        if(node->child)
        {
          node=node->child;
          continue;
        }
        return;
      }else
      {
        uint8_t off=node->BinFind(*key)-1;
        if(off>=0 && off<node->chSize && node->Children()[off]==*key)
        {
          node=node->childrenArr[off];
        }else
        {
          return;
        }
      }
      key++;
      if(!*key)
      {
        CopyData(node,values);
        return;
      }
    }
  }

protected:

  const char* getSig()
  {
    return "DXTR";
  }
  const uint16_t getVer()
  {
    return 0x0100;
  }

  struct DataListNode{
    DataListNode():next(0)
    {

    }
    DataListNode(const T& d,DataListNode* n):next(n),data(d),cnt(1)
    {
      if(n)
      {
        cnt=n->cnt+1;
      }
    }
    void Delete()
    {
      while(next)
      {
        DataListNode* ptr=next->next;
        delete next;
        next=ptr;
      }
    }
    void Write(File& f)
    {
      DataListNode* ptr=this;
      while(ptr)
      {
        XTWriteData(f,ptr->data);
        ptr=ptr->next;
      }
    }
    DataListNode* next;
    T data;
    uint32_t cnt;
    uint32_t Size()const
    {
      uint32_t rv=0;
      const DataListNode* ptr=this;
      while(ptr)
      {
        rv+=XTDataSize(ptr->data);
        ptr=ptr->next;
      }
      return rv;
    }
  };

  struct DataArrayList{
    DataArrayList():next(0),data(0),dataSize(0),dataCount(0){
    }

    DataArrayList(uint32_t sz):next(0),data(0),dataSize(sz),dataCount(0)
    {
      data=new T[sz];
    }

    void Clear()
    {
      DataArrayList* ptr=this,*toKill;
      while(ptr)
      {
        if(ptr->data)
        {
          delete [] ptr->data;
        }
        if(ptr!=this)
        {
          toKill=ptr;
          ptr=ptr->next;
          delete toKill;
        }else
        {
          ptr=ptr->next;
        }
      }
    }
    void Push(const T& value)
    {
      if(dataCount==dataSize)
      {
        DataArrayList* ptr=new DataArrayList;
        ptr->next=next;
        ptr->data=data;
        ptr->dataSize=dataSize;
        ptr->dataCount=dataCount;
        if(dataSize<1024)dataSize*=2;
        data=new T[dataSize];
        dataCount=0;
        next=ptr;
      }
      data[dataCount]=value;
      dataCount++;
    }
    uint32_t Size()const
    {
      uint32_t rv=0;
      const DataArrayList* ptr=this;
      while(ptr)
      {
        rv+=XTDataSize(ptr->data[0])*ptr->dataCount;
        ptr=ptr->next;
      }
      return rv;
    }
    void Write(File& f)
    {
      DataArrayList* ptr=this;
      while(ptr)
      {
        for(unsigned int i=0;i<ptr->dataCount;i++)
        {
          XTWriteData(f,ptr->data[i]);
        }
        ptr=ptr->next;
      }
    }
    DataArrayList* next;
    T* data;
    uint32_t dataSize;
    uint32_t dataCount;
  };

  enum NodeType
  {
    ntCollapsed,
    ntNormal,
    ntCollapsedWithChild//for disk only
  };
  enum DataType
  {
    dtSingleValue,
    dtLinkedList,
    dtArrayList
  };
  struct Node
  {
    union {
      char* children;
      char* str;
      char shortStr[8];
      char shortChildren[8];
    };
    union{
      Node** childrenArr;
      Node* child;
      File::offset_type childOff;
      File::offset_type* childsOff;
    };
    union{
      T* data;
      DataArrayList* dataArray;
      DataListNode* dataList;
      File::offset_type dataOff;
    };
    File::offset_type offset;
    uint8_t chSize;
    uint8_t nodeType;
    uint8_t dataType;
    bool real:1;
    bool heapStr:1;

    Node():children(0),childrenArr(0),data(0),chSize(0),nodeType(ntCollapsed),dataType(dtSingleValue),real(true),heapStr(false)
    {

    }
    ~Node()
    {
      Clear();
    }

    char* Str()
    {
      if(heapStr)return str;
      return shortStr;
    }
    char* Children()
    {
      if(heapStr)return children;
      return shortChildren;
    }

    void Clear()
    {
      if(nodeType==ntCollapsed)
      {
        if(str && heapStr)
        {
          //printf("[%p]delete str \"%s\"\n",this,str);
          delete [] str;
          str=0;
          heapStr=false;
        }
        if(child && real)
        {
          //printf("[%p]delete child %p\n",this,child);
          delete child;
        }
        child=0;
      }else //ntNormal
      {
        if(children && heapStr)
        {
          //printf("[%p]delete [] children \"%.*s\"\n",this,chSize,children);
          delete [] children;
          children=0;
          heapStr=false;
        }
        if(real)
        {
          for(int i=0;i<chSize;i++)
          {
            //printf("[%p]delete childrenArr[%d] %p\n",this,i,childrenArr[i]);
            delete childrenArr[i];
          }
          //printf("[%p]delete [] childrenArr %p\n",this,childrenArr);
          if(childrenArr)delete [] childrenArr;
          childrenArr=0;
        }else
        {
          if(childsOff)delete [] childsOff;
        }
      }
      if(data && real)
      {
        //printf("[%p]delete data %p\n",this,data);
        if(dataType==dtSingleValue)
        {
          delete data;
        }else
        if(dataType==dtLinkedList)
        {
          dataList->Delete();
          delete dataList;
        }else//dtArrayList
        {
          dataArray->Clear();
          delete dataArray;
        }
      }
      dataType=dtSingleValue;
      nodeType=ntCollapsed;
      data=0;
      chSize=0;
    }

    void StoreData(const T& value)
    {
      if(dataType==dtSingleValue)
      {
        if(!data)
        {
          data=new T(value);
        }else
        {
          T* ptr=data;
          dataList=new DataListNode(value,new DataListNode());
          dataList->next->data=*ptr;
          delete ptr;
          dataType=dtLinkedList;
        }
      }else if(dataType==dtLinkedList)
      {
        if(dataList->cnt<4)
        {
          dataList=new DataListNode(value,dataList);
        }else
        {
          DataListNode* lst=dataList,*ptr=dataList;
          dataArray=new DataArrayList(8);
          while(ptr)
          {
            dataArray->Push(ptr->data);
            ptr=ptr->next;
          }
          lst->Delete();
          delete lst;
          dataType=dtArrayList;
          dataArray->Push(value);
        }
      }else//dtArrayList
      {
        dataArray->Push(value);
      }
    }

    Node* Split(char* pos,NodeType nt,const char*& key)
    {
      if(*pos)
      {
        if(nt==ntCollapsed)
        {
          //printf("csplit:%s\n",pos);
          Node* nn=new Node();
          nn->nodeType=nt;
          size_t l=strlen(pos);
          if(l>7)
          {
            nn->str=new char[l+1];
            memcpy(nn->str,pos,l+1);
            nn->heapStr=true;
          }else
          {
            memcpy(nn->shortStr,pos,l+1);
          }
          nn->data=data;
          nn->dataType=dataType;
          nn->child=child;
          child=nn;
          data=0;
          dataType=dtSingleValue;
          *pos=0;
          return this;
        }else//ntNormal
        {
          if(pos!=Str())
          {
            //printf("msplit:%s,%s\n",pos,key);
            Node* nn=new Node();
            nn->nodeType=nt;
            nn->childrenArr=new Node*[2];
            int pidx=*pos<*key?0:1;
            int cidx=1-pidx;
            size_t l=strlen(pos+1);
            if(l>0 || !child || data)
            {
              nn->childrenArr[pidx]=new Node();
              nn->childrenArr[cidx]=new Node();
              if(l>7)
              {
                nn->childrenArr[pidx]->str=new char[l+1];
                memcpy(nn->childrenArr[pidx]->str,pos+1,l+1);
                nn->childrenArr[pidx]->heapStr=true;
              }else
              {
                memcpy(nn->childrenArr[pidx]->shortStr,pos+1,l+1);
              }
              nn->childrenArr[pidx]->data=data;
              nn->childrenArr[pidx]->dataType=dataType;
              nn->childrenArr[pidx]->child=child;
              data=0;
              dataType=dtSingleValue;
            }else
            {
              nn->childrenArr[cidx]=new Node();
              nn->childrenArr[pidx]=child;
            }
            nn->shortChildren[pidx]=*pos;
            *pos=0;
            /*
            char* newstr=new char[pos-str+1];
            memcpy(newstr,str,pos-str);
            newstr[pos-str]=0;
            delete [] str;
            str=newstr;
            */
            child=nn;
            nn->chSize=2;
            nn->shortChildren[cidx]=*key;
            key++;
            return nn->childrenArr[cidx];
          }else
          {
            //printf("psplit:%s,%s\n",str,key);
            Node* nn[2];//={new Node(),new Node()};
            size_t l=strlen(Str()+1);
            nodeType=ntNormal;
            int pidx=*pos<*key?0:1;
            int cidx=1-pidx;
            if(l>0 || !child || data)
            {
              nn[0]=new Node();
              nn[1]=new Node();
              if(l>7)
              {
                nn[pidx]->str=new char[l+1];
                memcpy(nn[pidx]->str,Str()+1,l+1);
                nn[pidx]->heapStr=true;
              }else
              {
                memcpy(nn[pidx]->shortStr,Str()+1,l+1);
              }
              nn[pidx]->child=child;
              nn[pidx]->data=data;
              nn[pidx]->dataType=dataType;
              data=0;
              dataType=dtSingleValue;
            }else
            {
              nn[pidx]=child;
              nn[cidx]=new Node();
            }
            char p=*Str();
            if(heapStr)delete [] str;
            //children=new char[2];
            heapStr=false;
            shortChildren[pidx]=p;
            shortChildren[cidx]=*key;
            childrenArr=new Node*[2];
            childrenArr[pidx]=nn[pidx];
            childrenArr[cidx]=nn[cidx];
            chSize=2;
            key++;
            return childrenArr[cidx];
          }
        }
      }
      if(child)return child;
      return child=new Node;
    }
    Node* FindInsert(char c,Node**& pptr)
    {
      int idx=BinFind(c);
      if(idx>0 && idx<=chSize && Children()[idx-1]==c)
      {
        pptr=&childrenArr[idx-1];
        return childrenArr[idx-1];
      }
      if(chSize+1<8)
      {
        Node** newchar=new Node*[chSize+1];
        memcpy(newchar,childrenArr,sizeof(Node*)*idx);
        memmove(shortChildren+idx+1,shortChildren+idx,chSize-idx);
        memcpy(newchar+idx+1,childrenArr+idx,sizeof(Node*)*(chSize-idx));
        delete [] childrenArr;
        childrenArr=newchar;
      }else
      {
        char* newch=new char[chSize+1];
        Node** newchar=new Node*[chSize+1];
        memcpy(newch,Children(),idx);
        memcpy(newchar,childrenArr,sizeof(Node*)*idx);
        memcpy(newch+idx+1,Children()+idx,chSize-idx);
        memcpy(newchar+idx+1,childrenArr+idx,sizeof(Node*)*(chSize-idx));
        if(heapStr)delete [] children;
        heapStr=true;
        delete [] childrenArr;
        children=newch;
        childrenArr=newchar;
      }
      Children()[idx]=c;
      childrenArr[idx]=new Node();
      chSize++;
      pptr=&childrenArr[idx];
      return childrenArr[idx];
    }
    int BinFind(char c)
    {
      int cnt=chSize;
      int cnt2;
      int off=0;
      char* ch=Children();
      while(cnt>0)
      {
        cnt2=cnt/2;
        if(!(c<ch[off+cnt2]))
        {
          off+=cnt2+1;
          cnt-=cnt2+1;
        }else
        {
          if(ch[off+cnt2]==c)return off+cnt2;
          cnt=cnt2;
        }
      }
      return off;
    }
    static void Write40BitOffset(File& f,File::offset_type off)
    {
      if(off&0xffffff0000000000ull)
      {
        fprintf(stderr,"Attempt to write 40bit offset that exceed range:%llx\n",off);
        abort();
      }
      uint8_t high=(off&0x000000ff00000000ull)>>32;
      uint32_t low=off&0xffffffff;
      f.WriteByte(high);
      f.WriteNetInt32(low);
    }
    static File::offset_type Read40BitOffset(File& f)
    {
      File::offset_type rv;
      uint8_t high=f.ReadByte();
      uint32_t low=f.ReadNetInt32();
      rv=high;
      rv<<=32;
      rv|=low;
      return rv;
    }
    size_t NodeSize()
    {
      size_t rv=1/*nodeType*/+1/*strlen*/;
      if(nodeType==ntNormal)
      {
        rv+=chSize;
      }else
      {
        rv+=Str()?strlen(Str()):0;
      }
      rv+=4/*datasize*/;
      if(dataType==dtSingleValue)
      {
        if(data)rv+=XTDataSize(*data);
      }else if(dataType==dtLinkedList)
      {
        rv+=dataList->Size();
      }else//dtArrayList
      {
        rv+=dataArray->Size();
      }
      rv+=(nodeType==ntNormal?5*chSize:child?5:0)/*child/children offsets*/;
      return rv;
    }
    void Write(File& f)
    {
      uint8_t nt=nodeType;
      if(nodeType==ntCollapsed && child)
      {
        nt=ntCollapsedWithChild;
      }
      f.WriteByte(nt);
      if(nodeType==ntNormal)
      {
        f.WriteByte(chSize);
        f.Write(Children(),chSize);
      }else
      {
        char* s=Str();
        uint8_t l=s?strlen(s):0;
        f.WriteByte(l);
        if(l)f.Write(s,l);
      }
      uint32_t dsz;
      if(dataType==dtSingleValue)
      {
        dsz=data?XTDataSize(*data):0;
        f.WriteNetInt32(dsz);
        if(data)XTWriteData(f,*data);
      }else if(dataType==dtLinkedList)
      {
        dsz=dataList->Size();
        f.WriteNetInt32(dsz);
        dataList->Write(f);
      }else//dtArrayList
      {
        dsz=dataArray->Size();
        f.WriteNetInt32(dsz);
        dataArray->Write(f);
      }

      if(nodeType==ntCollapsed)
      {
        if(child)
        {
          Write40BitOffset(f,child->offset);
          child->Write(f);
        }
      }else
      {
        for(int i=0;i<chSize;i++)
        {
          Write40BitOffset(f,childrenArr[i]->offset);
        }
        for(int i=0;i<chSize;i++)
        {
          childrenArr[i]->Write(f);
        }
      }
    }
    void Read(File& f,bool recursive)
    {
      real=recursive;
      uint8_t nt=f.ReadByte();
      nodeType=nt==ntNormal?ntNormal:ntCollapsed;
      uint8_t l=f.ReadByte();
      if(nodeType==ntNormal)
      {
        chSize=l;
        if(l>8)
        {
          children=new char[l];
          heapStr=true;
        }
        f.Read(Children(),l);
      }else
      {
        if(l>7)
        {
          str=new char[l+1];
          heapStr=true;
        }
        f.Read(Str(),l);
        Str()[l]=0;
      }
      if(recursive)
      {
        uint32_t dsz=f.ReadNetInt32();
        T value;
        while(dsz>0)
        {
          dsz-=XTReadData(f,value);
          StoreData(value);
        }
      }else
      {
        dataOff=f.Pos();
        uint32_t dsz=f.ReadNetInt32();
        f.SeekCur(dsz);
      }
      if(!recursive)
      {
        childOff=0;
      }
      if(nt==ntCollapsedWithChild)
      {
        if(recursive)
        {
          child=new Node();
          Read40BitOffset(f);
          child->Read(f,true);
        }else
        {
          childOff=Read40BitOffset(f);
        }
      }else if(nt==ntNormal)
      {
        if(recursive)
        {
          childrenArr=new Node*[chSize];
          for(int i=0;i<chSize;i++)
          {
            childrenArr[i]=new Node();
            Read40BitOffset(f);
          }
          for(int i=0;i<chSize;i++)
          {
            childrenArr[i]->Read(f,true);
          }
        }else
        {
          childsOff=new File::offset_type[chSize];
          for(int i=0;i<chSize;i++)
          {
            childsOff[i]=Read40BitOffset(f);
          }
        }
      }
    }
  };

  static void ReadData(File& f,File::offset_type off,std::vector<T>& values)
  {
    f.Seek(off);
    uint32_t dsz=f.ReadNetInt32();
    while(dsz>0)
    {
      T v;
      dsz-=XTReadData(f,v);
      values.push_back(v);
    }
  }

  void CopyData(Node* node,std::vector<T>& values)
  {
    if(node->dataType==dtSingleValue)
    {
      if(node->data)
      {
        values.push_back(*node->data);
      }
    }else if(node->dataType==dtLinkedList)
    {
      DataListNode* ptr=node->dataList;
      while(ptr)
      {
        values.push_back(ptr->data);
        ptr=ptr->next;
      }
    }else//dtArrayList
    {
      DataArrayList* ptr=node->dataArray;
      while(ptr)
      {
        values.insert(values.end(),ptr->data,ptr->data+ptr->dataCount);
        ptr=ptr->next;
      }
    }
  }

  Node root;

  void PrepareOffsets(Node* node,File::offset_type& offset)
  {
    node->offset=offset;
    offset+=node->NodeSize();
    if(node->nodeType==ntNormal)
    {
      for(int i=0;i<node->chSize;i++)
      {
        PrepareOffsets(node->childrenArr[i],offset);
      }
    }else
    {
      if(node->child)
      {
        PrepareOffsets(node->child,offset);
      }
    }
  }

  void DumpValue(Node* node)
  {
    printf(" value=\"");
    if(node->dataType==dtSingleValue)
    {
      XTPrintData(*node->data);
    }else if(node->dataType==dtLinkedList)
    {
      DataListNode* l=node->dataList;
      while(l)
      {
        XTPrintData(l->data);
        if(l->next)printf(",");
        l=l->next;
      }
    }else//dtArrayList
    {
      DataArrayList* ptr=node->dataArray;
      unsigned int initValue=1;
      XTPrintData(ptr->data[0]);
      while(ptr)
      {
        for(unsigned int i=initValue;i<ptr->dataCount;i++)
        {
          printf(",");
          XTPrintData(ptr->data[i]);
        }
        ptr=ptr->next;
        initValue=0;
      }
    }
    printf("\"");
  }
  void Dump(Node* ptr,int shift,char c=0)
  {
    if(!ptr)return;
    printf("%*s",shift,"");
    //printf("<node size=\"%d\" offset=\"%lld\"",ptr->NodeSize(),ptr->offset);
    printf("<node type=\"%c\"",ptr->nodeType==ntCollapsed?'c':'n');
    if(ptr->nodeType==ntCollapsed)
    {
      if(c)printf(" char=\"%c\"",c);
      printf(" str=\"%s\"",ptr->Str()?ptr->Str():"");
      if(ptr->data)DumpValue(ptr);
      printf(">\n");
      Dump(ptr->child,shift+2);
      printf("%*s</node>\n",shift,"");
    }else
    {
      if(c)printf(" char=\"%c\"",c);
      printf(" children=\"%.*s\"",ptr->chSize,ptr->Children());

      if(ptr->data)DumpValue(ptr);

      printf(">\n");
      for(int i=0;i<ptr->chSize;i++)
      {
        Dump(ptr->childrenArr[i],shift+2,ptr->Children()[i]);
      }
      printf("%*s</node>\n",shift,"");
    }
  }

};

}
}
}

#endif

