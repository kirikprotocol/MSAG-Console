#ifndef __SMSC_UTIL_64BITCOMPAT_H__
#define __SMSC_UTIL_64BITCOMPAT_H__

inline int VoidPtr2Int(void* arg)
{
  if(sizeof(arg)==4)
  {
    int* ptr=(int*)&arg;
    return *ptr;
  }else
  {
    long long* ptr=(long long*)&arg;
    return *ptr;
  }
}


#endif
