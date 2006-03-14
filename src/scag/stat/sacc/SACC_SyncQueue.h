#ifndef _SACC_Queue_H_
#define _SACC_Queue_H_

/*****************************************************************************
 *****************************************************************************
 *																			 
 * Module Name:																  
 * ============
 *
 *			SACC_Queue.h
 *
 * Purpose:			
 * ========
 *			Syncronized queue for event trnslating to sender thread
 *			
 * Author(s) & Creation Date:	
 * ==========================
 *
 *			Gregory Panin, 2006/03/03
 *  Copyright (c): 
 *  =============
 * 			EyeLine Communications
 * 			All rights reserved.
 */

#include <core/buffers/FastMTQueue.hpp>
#include <core/synchronization/EventMonitor.hpp>
#include <core/synchronization/Event.hpp>

using namespace smsc::core::buffers;
using namespace smsc::core::synchronization;

namespace scag{
namespace stat{
namespace sacc{

template <class T>
class SyncQeuue{
public:
 void Push(const T& item)
 {

  MutexGuard g(mtx);
  q.Push(item);
  mtx.notify();

 };
 bool Pop(T& item,int timeout)
 {
  MutexGuard g(mtx);
  int ret=0;
  while(!q.Pop(item))
  {
   ret = mtx.wait(timeout);
   if(ret!=0)
    return false;
  }
  
  return true;
  
 };
 void Wait()
 {
  MutexGuard g(mtx);
  mtx.wait();
 };
 int Count()
 {
  MutexGuard g(mtx);
  return q.Count();
 };
protected:
 CyclicQueue<T> q;
 EventMonitor mtx;
 
};


}//scag
}//stat
}//sacc
#endif