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
#include <vector>
#include <stdio.h>

using namespace smsc::core::buffers;
using namespace smsc::core::synchronization;
using namespace std;

namespace scag{
namespace stat{
namespace sacc{

template <class T>
class SyncQueue{
public:
 int Push(const T& item,int maxsize)
 {

  MutexGuard g(mtx);

  if(q.Count()<maxsize)
  {
	  q.Push(item);
	  mtx.notify();
  }
  else
  {
	  return -1;
  }
  
  return 1;
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
 //void notify()
 //{
//	mtx.notify();
// };
protected:
 CyclicQueue<T> q;
 EventMonitor mtx;
 
};

struct PDUPair{
uint8_t *msg_p;
uint32_t size;
	PDUPair()
	{
		msg_p=0;
		size=0;	
	}

	PDUPair(const PDUPair & src)
	{
		msg_p = new uint8_t[src.size];
		memcpy(msg_p,src.msg_p,src.size);
		size=src.size;	
	}


};


class SaccPDU{
public:
	
	SaccPDU()
	{
		PDUPair hdr;
		hdr.msg_p = new uint8_t[sizeof(uint32_t)+sizeof(uint16_t)];
		hdr.size  = sizeof(uint32_t)+sizeof(uint16_t);
		nodes.push_back(hdr);
	};
 
	~SaccPDU()
	{
		nodes.clear();
	};

public:
 std::vector<PDUPair>nodes;
 EventMonitor mtx;

 uint32_t getallsize()
 {
  uint32_t res=0;
  for(int i=0;i<nodes.size();i++)
   res+=nodes[i].size;
  ////xmap_trace(logger," get all size = %d",res);
  return res;
 };

 void insertPDUString8(uint8_t * ptr,int maximum)
 {
	MutexGuard g(mtx);

	uint32_t segmentsize=0;

	while(ptr[segmentsize]!=0 && segmentsize <  maximum )
	{
		segmentsize++;
	}
	if(segmentsize)
	{
	  PDUPair mPair;

	  mPair.size = segmentsize+sizeof(uint16_t);
	  mPair.msg_p= new uint8_t[segmentsize+sizeof(uint16_t)];//(UCHAR_T*)malloc(size);
  	  mPair.msg_p[0]=(uint8_t)((segmentsize &0xff00)>>8);
	  mPair.msg_p[1]=(uint8_t)( segmentsize &0x00ff);
	  memcpy(mPair.msg_p+2,ptr,segmentsize);
	  nodes.push_back(mPair);
	  
	}

 }

 void insertPDUString16(uint16_t * ptr,int maximum)
 {
	MutexGuard g(mtx);

	uint32_t segmentsize=wcslen((wchar_t*)ptr);


/*	while((uint16_t)(ptr[segmentsize])!=0 && segmentsize <  maximum )
	{
		segmentsize++;
	}
*/
	//ssz=16
	if(segmentsize)
	{
	  PDUPair mPair;

	  mPair.size = segmentsize*sizeof(uint16_t)+sizeof(uint16_t);
	  
	  mPair.msg_p= new uint8_t[mPair.size];//(UCHAR_T*)malloc(size);
  	  mPair.msg_p[0]=(uint8_t)((segmentsize &0xff00)>>8);
	  mPair.msg_p[1]=(uint8_t)( segmentsize &0x00ff);
	  
	  memcpy(mPair.msg_p+2,(uint8_t*)ptr,mPair.size);

	  nodes.push_back(mPair);
	  
	}

 }
 void insertSegment(uint8_t* ptr,uint16_t size)
 {
	  MutexGuard g(mtx);

	  PDUPair mPair;
                
	  mPair.size = size+sizeof(uint16_t);
	  mPair.msg_p= new uint8_t[size+sizeof(uint16_t)];//(UCHAR_T*)malloc(size);
	  
	  mPair.msg_p[0]=(uint8_t)((size &0xff00)>>8);
	  mPair.msg_p[1]=(uint8_t)( size &0x00ff);

	  memcpy(mPair.msg_p+2,ptr,size);

	  nodes.push_back(mPair);
 };

 void insertPDUHeader(uint16_t cid)
 {
	MutexGuard g(mtx);

	uint32_t pdulen = getallsize();
	nodes[0].msg_p[0] = (uint8_t)((pdulen & 0xff000000)>>24);
	nodes[0].msg_p[1] = (uint8_t)((pdulen & 0x00ff0000)>>16);
	nodes[0].msg_p[2] = (uint8_t)((pdulen & 0x0000ff00)>>8);
	nodes[0].msg_p[3] = (uint8_t)(pdulen & 0x000000ff);
	nodes[0].msg_p[4]=(uint8_t)((cid &0xff00)>>8);
	nodes[0].msg_p[5]=(uint8_t)( cid &0x00ff);
 }
 
 void getAll(uint8_t *ptr)
 {
	  MutexGuard g(mtx);
	  if(ptr!=0)
	  {
	   for(int i=0;i<nodes.size();i++)
	   {
		   if(nodes[i].size)
		   {
			   memcpy(ptr,nodes[i].msg_p,nodes[i].size);
			   ptr+=nodes[i].size;
		   }

		   
	   }
	  }
 };

 void free()
 {
  MutexGuard g(mtx);
  for(int i=0;i<nodes.size();i++)
  {
   if(nodes[i].msg_p)
    delete nodes[i].msg_p;
  }
  nodes.empty();
 };

};



}//scag
}//stat
}//sacc
#endif
