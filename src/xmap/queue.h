#ifndef _QUEUE_H_GREEN_
#define _QUEUE_H_GREEN_

#include <list>
#include <thread.h>
#include <synch.h>
#include <queue>
#include "common.h"
#include "xmap.h"



class MMAP_MSG{
public:
 MMAP_MSG();
  MMAP_MSG(const MMAP_MSG& h);
 MMAP_MSG(MMAP_MSG* h);
 MMAP_MSG(const MSG_T message);
 MMAP_MSG(MSG_T *message);
 MMAP_MSG(USHORT_T sndr, USHORT_T rcvr,UCHAR_T prim,USHORT_T sz,UCHAR_T *ptr);

 ~MMAP_MSG();
 MMAP_MSG operator = (const MMAP_MSG & other_);


 void toMsg_t(MSG_T*message);
public:
 USHORT_T sender;
 USHORT_T receiver;
 UCHAR_T primitive;
 USHORT_T size;
 UCHAR_T *msg_p;
 
};


class cqueue{

public:
 std::list<MMAP_MSG> msgQueue;
 std::queue<int> mq;

 pthread_mutex_t mutex;
 int signaled;
public:
 cqueue();
 ~cqueue();
public:
 void put(MMAP_MSG mh);
 //MMAP_MSG get();
 bool get(MMAP_MSG & item);
 int getSize();   
};


 
 

#endif