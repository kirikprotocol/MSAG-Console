#ifndef __REFPTR_HPP__
#define __REFPTR_HPP__

namespace smsc{
namespace core{
namespace buffers{

class EmptyMutex{
public:
  void Lock(){}
  void Unlock(){}
};

template <class T,class S=EmptyMutex>
class RefPtr{
protected:
  struct RefPtrData{
    mutable S lock;
    RefPtrData():refCount(0),ptr(0)
    {
    }
    mutable int refCount;
    T  *ptr;
    void Lock()const
    {
      lock.Lock();
    }
    void Unlock()const
    {
      lock.Unlock();
    }
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
    src.Ref();
    data=src.data;
  }
  ~RefPtr()
  {
    Unref();
  }
  RefPtr& operator=(const RefPtr& src)
  {
    Unref();
    src.Ref();
    data=src.data;
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
  void Ref()const
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
