#ifndef __REFPTR_HPP__
#define __REFPTR_HPP__

namespace smsc{
namespace core{
namespace buffers{

template <class T>
class RefPtr{
protected:
  struct RefPtrData{
    RefPtrData():refCount(0),ptr(0)
    {
    }
    int refCount;
    T  *ptr;
    void Lock(){}
    void Unlock(){}
  };
public:
  explicit RefPtr(T* ptr=NULL)
  {
    data=new RefPtrData;
    data->ptr=ptr;
    Ref();
  }
  RefPtr(const RefPtr& src)
  {
    data=src.data;
    Ref();
  }
  ~RefPtr()
  {
    Unref();
  }
  RefPtr& operator=(const RefPtr& src)
  {
    Unref();
    data=src.data;
    Ref();
    return *this;
  }
  RefPtr& operator=(T* ptr)
  {
    Unref();
    data=new RefPtrData;
    data->ptr=ptr;
    Ref();
    return *this;
  }
  T& operator*()
  {
    return *data->ptr;
  }
  T* operator->()
  {
    return data->ptr;
  }
  T* Get()
  {
    return data->ptr;
  }
protected:
  void Ref()
  {
    data->Lock();
    data->refCount++;
    data->Unlock();
  }
  void Unref()
  {
    if(!data || !data->refCount)return;
    data->Lock();
    int count=--data->refCount;
    data->Unlock();
    if(!count)
    {
      if(data->ptr)delete data->ptr;
      delete data;
    }
  }
  RefPtrData *data;
};//RefPtr

}//namespace buffers
}//namespace core
}//namespace smsc

#endif
