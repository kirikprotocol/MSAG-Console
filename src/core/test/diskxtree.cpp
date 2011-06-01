/* "@(#)$Id$" */

#include <stdio.h>
#define NOLOGGERPLEASE 1

#include "util/int.h"
#include "util/Timer.hpp"
#include "core/buffers/DiskXTree.hpp"
#include "sms/sms.h"
#include "core/buffers/TmpBuf.hpp"
#include "util/Uint64Converter.h"

#ifndef NOLOGGERPLEASE
#include "logger/Logger.h"
#endif

using namespace smsc::core::buffers;
using namespace smsc::sms;
using smsc::util::Uint64Converter;
using smsc::util::Exception;

File storeFile;

void sopen(const char* fn)
{
  storeFile.ROpen(fn);
  char magic[9]={0,};
  storeFile.Read(magic,8);
  if(strcmp(magic,"SMSC.ARC")!=0)
  {
    throw std::runtime_error("bad file magic");
  }
  uint16_t ver=storeFile.ReadNetInt16();
  if(ver!=1)
  {
    throw std::runtime_error("bad file version");
  }
}

void swrite(void* data,uint32_t sz)
{
  storeFile.Write(data,sz);
}

bool sread(void* data,uint32_t sz)
{
  storeFile.Read(data,sz);
  return true;
}

/*
void ssave(SMSId id, SMS& sms)
{
    uint8_t smsState = (uint8_t)sms.state;
    std::string oa  = sms.originatingAddress.toString();
    std::string da  = sms.destinationAddress.toString();
    std::string dda = sms.dealiasedDestinationAddress.toString();
    int8_t oaSize   = oa.length();
    int8_t daSize   = da.length();
    int8_t ddaSize  = dda.length();
    int8_t svcSize    = strlen(sms.eServiceType);
    int8_t odMscSize  = strlen(sms.originatingDescriptor.msc);
    int8_t odImsiSize = strlen(sms.originatingDescriptor.imsi);
    int8_t ddMscSize  = strlen(sms.destinationDescriptor.msc);
    int8_t ddImsiSize = strlen(sms.destinationDescriptor.imsi);
    int8_t routeSize  = strlen(sms.routeId);
    int8_t srcSmeSize = strlen(sms.srcSmeId);
    int8_t dstSmeSize = strlen(sms.dstSmeId);
    int32_t bodyBufferLen = sms.messageBody.getBufferLength();
    int32_t textLen       = 0;

    uint32_t recordSize = sizeof(id)+sizeof(smsState)+sizeof(uint32_t)+sizeof(uint32_t)+
        sizeof(sms.attempts)+sizeof(sms.lastResult)+sizeof(uint32_t)+sizeof(uint32_t)+
        sizeof(oaSize)+oaSize+sizeof(daSize)+daSize+sizeof(ddaSize)+ddaSize+sizeof(sms.messageReference)+
        sizeof(svcSize)+svcSize+sizeof(sms.deliveryReport)+sizeof(sms.billingRecord)+
        sizeof(odMscSize)+odMscSize+sizeof(odImsiSize)+odImsiSize+sizeof(sms.originatingDescriptor.sme)+
        sizeof(ddMscSize)+ddMscSize+sizeof(ddImsiSize)+ddImsiSize+sizeof(sms.destinationDescriptor.sme)+
        sizeof(routeSize)+routeSize+sizeof(sms.serviceId)+sizeof(sms.priority)+
        sizeof(srcSmeSize)+srcSmeSize+sizeof(dstSmeSize)+dstSmeSize+
        sizeof(bodyBufferLen)+bodyBufferLen;

    uint32_t writeBufferSize = recordSize+sizeof(recordSize)*2;
    TmpBuf<char, 2048> writeBufferGuard(writeBufferSize);
    char* writeBuffer = writeBufferGuard.get(); char* position = writeBuffer;

    recordSize = htonl(recordSize);
    memcpy(position, &recordSize, sizeof(recordSize)); position+=sizeof(recordSize);
    SMSId idd = Uint64Converter::toNetworkOrder(id);
    memcpy(position, &idd, sizeof(idd)); position+=sizeof(idd);
    memcpy(position, &smsState, sizeof(smsState)); position+=sizeof(smsState);

    uint32_t writeTime = htonl(sms.submitTime);
    memcpy(position, &writeTime, sizeof(writeTime)); position+=sizeof(writeTime);
    writeTime = htonl(sms.validTime);
    memcpy(position, &writeTime, sizeof(writeTime)); position+=sizeof(writeTime);
    writeTime = htonl(sms.lastTime);
    memcpy(position, &writeTime, sizeof(writeTime)); position+=sizeof(writeTime);
    writeTime = htonl(sms.nextTime);
    memcpy(position, &writeTime, sizeof(writeTime)); position+=sizeof(writeTime);

    uint32_t attempts = htonl(sms.attempts);
    memcpy(position, &attempts, sizeof(attempts)); position+=sizeof(attempts);
    uint32_t lastResult = htonl(sms.lastResult);
    memcpy(position, &lastResult, sizeof(lastResult)); position+=sizeof(lastResult);

    memcpy(position, &oaSize, sizeof(oaSize));   position+=sizeof(oaSize);
    if (oaSize > 0)  { memcpy(position, oa.c_str(), oaSize);   position+=oaSize;  }
    memcpy(position, &daSize, sizeof(daSize));   position+=sizeof(daSize);
    if (daSize > 0)  { memcpy(position, da.c_str(), daSize);   position+=daSize;  }
    memcpy(position, &ddaSize, sizeof(ddaSize)); position+=sizeof(ddaSize);
    if (ddaSize > 0) { memcpy(position, dda.c_str(), ddaSize); position+=ddaSize; }

    uint16_t mr = htons(sms.messageReference);
    memcpy(position, &mr, sizeof(mr)); position+=sizeof(mr);
    memcpy(position, &svcSize, sizeof(svcSize)); position+=sizeof(svcSize);
    if (svcSize > 0) { memcpy(position,  sms.eServiceType, svcSize); position+=svcSize; }
    memcpy(position, &sms.deliveryReport, sizeof(sms.deliveryReport)); position+=sizeof(sms.deliveryReport);
    memcpy(position, &sms.billingRecord, sizeof(sms.billingRecord)); position+=sizeof(sms.billingRecord);

    memcpy(position, &odMscSize, sizeof(odMscSize));   position+=sizeof(odMscSize);
    if (odMscSize > 0)  { memcpy(position, sms.originatingDescriptor.msc, odMscSize);   position+=odMscSize; }
    memcpy(position, &odImsiSize, sizeof(odImsiSize)); position+=sizeof(odImsiSize);
    if (odImsiSize > 0) { memcpy(position, sms.originatingDescriptor.imsi, odImsiSize); position+=odImsiSize; }
    uint32_t odSme = htonl(sms.originatingDescriptor.sme);
    memcpy(position, &odSme, sizeof(odSme)); position+=sizeof(odSme);
    memcpy(position, &ddMscSize, sizeof(ddMscSize));   position+=sizeof(ddMscSize);
    if (ddMscSize > 0)  { memcpy(position, sms.destinationDescriptor.msc, ddMscSize);   position+=ddMscSize; }
    memcpy(position, &ddImsiSize, sizeof(ddImsiSize)); position+=sizeof(ddImsiSize);
    if (ddImsiSize > 0) { memcpy(position, sms.destinationDescriptor.imsi, ddImsiSize); position+=ddImsiSize; }
    uint32_t ddSme = htonl(sms.destinationDescriptor.sme);
    memcpy(position, &ddSme, sizeof(ddSme)); position+=sizeof(ddSme);

    memcpy(position, &routeSize, sizeof(routeSize)); position+=sizeof(routeSize);
    if (routeSize > 0)  { memcpy(position,  sms.routeId, routeSize); position+=routeSize; }
    int32_t svcId = (int32_t)htonl(sms.serviceId);
    memcpy(position, &svcId, sizeof(svcId)); position+= sizeof(svcId);
    int32_t priority = (int32_t)htonl(sms.priority);
    memcpy(position, &priority, sizeof(priority)); position+=sizeof(priority);
    memcpy(position, &srcSmeSize, sizeof(srcSmeSize)); position+=sizeof(srcSmeSize);
    if (srcSmeSize > 0) { memcpy(position,  sms.srcSmeId, srcSmeSize); position+=srcSmeSize; }
    memcpy(position, &dstSmeSize, sizeof(dstSmeSize)); position+=sizeof(dstSmeSize);
    if (dstSmeSize > 0) { memcpy(position,  sms.dstSmeId, dstSmeSize); position+=dstSmeSize; }

    textLen = (int32_t)htonl(bodyBufferLen);
    memcpy(position, &textLen, sizeof(textLen)); position+=sizeof(textLen);
    if (bodyBufferLen > 0) {
        uint8_t* bodyBuffer = sms.messageBody.getBuffer();
        memcpy(position,  bodyBuffer, bodyBufferLen); position+=bodyBufferLen;
    }

    memcpy(position, &recordSize, sizeof(recordSize)); position+=sizeof(recordSize);

    swrite(writeBuffer, writeBufferSize);
}
*/

bool sload(SMSId& id, SMS& sms)
{
    uint8_t  smsState = 0;
    uint32_t recordSize1 = 0; uint32_t recordSize2 = 0;
    int8_t oaSize    = 0; int8_t daSize = 0; int8_t ddaSize  = 0; int8_t svcSize  = 0;
    int8_t odMscSize = 0; int8_t odImsiSize = 0;
    int8_t ddMscSize = 0; int8_t ddImsiSize = 0;
    int8_t routeSize = 0; int8_t srcSmeSize = 0; int8_t dstSmeSize = 0;
    //int32_t textLen  = 0; 
    int32_t bodyBufferLen = 0;

        if (!sread(&recordSize1, (uint32_t)sizeof(recordSize1))) return false;
        else recordSize1 = ntohl(recordSize1);

        TmpBuf<char, 2048> readBufferGuard(recordSize1);
        char* readBuffer = readBufferGuard.get();
        if (!sread(readBuffer, recordSize1)) return false;

        if (!sread(&recordSize2, (uint32_t)sizeof(recordSize2))) return false;
        else recordSize2 = ntohl(recordSize2);

        if (recordSize1 != recordSize2)
            throw Exception("Inconsistent archive file rs1=%u, rs2=%u", recordSize1, recordSize2);

        char* position = readBuffer;
        memcpy(&id, position, sizeof(id)); position+=sizeof(id);
        id = Uint64Converter::toHostOrder(id);
        memcpy(&smsState, position, sizeof(smsState)); position+=sizeof(smsState);
        sms.state = (smsc::sms::State)smsState;

        uint32_t rdTime;
        memcpy(&rdTime, position, sizeof(rdTime)); position+=sizeof(rdTime);
        sms.submitTime = ntohl(rdTime);
        memcpy(&rdTime,  position, sizeof(rdTime) ); position+=sizeof(rdTime);
        sms.validTime  = ntohl(rdTime);
        memcpy(&rdTime,   position, sizeof(rdTime)  ); position+=sizeof(rdTime);
        sms.lastTime   = ntohl(rdTime);
        memcpy(&rdTime,   position, sizeof(rdTime)  ); position+=sizeof(rdTime);
        sms.nextTime   = ntohl(rdTime);

        memcpy(&sms.attempts, position, sizeof(sms.attempts)); position+=sizeof(sms.attempts);
        sms.attempts = ntohl(sms.attempts);
        memcpy(&sms.lastResult, position, sizeof(sms.lastResult)); position+=sizeof(sms.lastResult);
        sms.lastResult = ntohl(sms.lastResult);

        char strBuff[1024];
        memcpy(&oaSize, position, sizeof(oaSize)); position+=sizeof(oaSize);
        if (oaSize > 0) {
            memcpy(strBuff, position, oaSize); strBuff[oaSize] = '\0';
            sms.originatingAddress = Address(strBuff);
            position+=oaSize;
        } else throw Exception("OA invalid, size=%d", oaSize);
        memcpy(&daSize, position, sizeof(daSize)); position+=sizeof(daSize);
        if (daSize > 0) {
            memcpy(strBuff, position, daSize); strBuff[daSize] = '\0';
            sms.destinationAddress = Address(strBuff);
            position+=daSize;
        } else throw Exception("DA invalid, size=%d", daSize);
        memcpy(&ddaSize, position, sizeof(ddaSize)); position+=sizeof(ddaSize);
        if (ddaSize > 0) {
            memcpy(strBuff, position, ddaSize); strBuff[ddaSize] = '\0';
            sms.dealiasedDestinationAddress = Address(strBuff);
            position+=ddaSize;
        } else throw Exception("DDA invalid, size=%d", ddaSize);

        memcpy(&sms.messageReference, position, sizeof(sms.messageReference));
        position+=sizeof(sms.messageReference);
        sms.messageReference = ntohs(sms.messageReference);

        memcpy(&svcSize, position, sizeof(svcSize)); position+=sizeof(svcSize);
        if (svcSize > 0) {
            if ((unsigned)svcSize <= sizeof(sms.eServiceType)) {
                memcpy(strBuff, position, svcSize); strBuff[svcSize] = '\0';
                sms.setEServiceType(strBuff);
                position+=svcSize;

            } else throw Exception("svcType invalid, size=%d", svcSize);
        }

        memcpy(&sms.deliveryReport, position, sizeof(sms.deliveryReport));
        position+=sizeof(sms.deliveryReport);
        memcpy(&sms.billingRecord, position, sizeof(sms.billingRecord));
        position+=sizeof(sms.billingRecord);

        memcpy(&odMscSize, position, sizeof(odMscSize)); position+=sizeof(odMscSize);
        if (odMscSize > 0) {
            memcpy(strBuff, position, odMscSize); strBuff[odMscSize] = '\0';
            sms.originatingDescriptor.setMsc(odMscSize, strBuff);
            position+=odMscSize;
        } else sms.originatingDescriptor.setMsc(0, "");
        memcpy(&odImsiSize, position, sizeof(odImsiSize)); position+=sizeof(odImsiSize);
        if (odImsiSize > 0) {
            memcpy(strBuff, position, odImsiSize); strBuff[odImsiSize] = '\0';
            sms.originatingDescriptor.setImsi(odImsiSize, strBuff);
            position+=odImsiSize;
        } else sms.originatingDescriptor.setImsi(0, "");
        memcpy(&sms.originatingDescriptor.sme, position, sizeof(sms.originatingDescriptor.sme));
        position+=sizeof(sms.originatingDescriptor.sme);
        sms.originatingDescriptor.sme = ntohl(sms.originatingDescriptor.sme);

        memcpy(&ddMscSize, position, sizeof(ddMscSize)); position+=sizeof(ddMscSize);
        if (ddMscSize > 0) {
            memcpy(strBuff, position, ddMscSize); strBuff[ddMscSize] = '\0';
            sms.destinationDescriptor.setMsc(ddMscSize, strBuff);
            position+=ddMscSize;
        } else sms.destinationDescriptor.setMsc(0, "");
        memcpy(&ddImsiSize, position, sizeof(ddImsiSize)); position+=sizeof(ddImsiSize);
        if (ddImsiSize > 0) {
            memcpy(strBuff, position, ddImsiSize); strBuff[ddImsiSize] = '\0';
            sms.destinationDescriptor.setImsi(ddImsiSize, strBuff);
            position+=ddImsiSize;
        } else sms.destinationDescriptor.setImsi(0, "");
        memcpy(&sms.destinationDescriptor.sme, position, sizeof(sms.destinationDescriptor.sme));
        position+=sizeof(sms.destinationDescriptor.sme);
        sms.destinationDescriptor.sme = ntohl(sms.destinationDescriptor.sme);

        memcpy(&routeSize, position, sizeof(routeSize)); position+=sizeof(routeSize);
        if (routeSize > 0) {
            memcpy(strBuff, position, routeSize); strBuff[routeSize] = '\0';
            sms.setRouteId(strBuff);
            position+=routeSize;
        } else sms.setRouteId("");

        memcpy(&sms.serviceId, position, sizeof(sms.serviceId)); position+=sizeof(sms.serviceId);
        sms.serviceId = (int32_t)ntohl(sms.serviceId);
        memcpy(&sms.priority, position, sizeof(sms.priority)); position+=sizeof(sms.priority);
        sms.priority = (int32_t)ntohl(sms.priority);

        memcpy(&srcSmeSize, position, sizeof(srcSmeSize)); position+=sizeof(srcSmeSize);
        if (srcSmeSize > 0) {
            memcpy(strBuff, position, srcSmeSize); strBuff[srcSmeSize] = '\0';
            sms.setSourceSmeId(strBuff);
            position+=srcSmeSize;
        } else sms.setSourceSmeId("");
        memcpy(&dstSmeSize, position, sizeof(dstSmeSize)); position+=sizeof(dstSmeSize);
        if (dstSmeSize > 0) {
            memcpy(strBuff, position, dstSmeSize); strBuff[dstSmeSize] = '\0';
            sms.setDestinationSmeId(strBuff);
            position+=dstSmeSize;
        } else sms.setDestinationSmeId("");

        memcpy(&bodyBufferLen, position, sizeof(bodyBufferLen)); position+=sizeof(bodyBufferLen);
        bodyBufferLen = (int32_t)ntohl(bodyBufferLen);
        if (bodyBufferLen > 0) {
            uint8_t* bodyBuffer = new uint8_t[bodyBufferLen];
            memcpy(bodyBuffer, position, bodyBufferLen); position+=bodyBufferLen;
            sms.messageBody.setBuffer(bodyBuffer, bodyBufferLen);
        } else sms.messageBody.setBuffer(0, 0);

    return storeFile.Pos()!=storeFile.Size();
}

struct IdLtt{
  uint64_t offset;
  uint32_t ltt;
};


size_t XTDataSize(IdLtt data)
{
  return 12;
}

void XTWriteData(smsc::core::buffers::File& file,const IdLtt& data)
{
  file.WriteNetInt64(data.offset);
  file.WriteNetInt32(data.ltt);
}

File::offset_type XTReadData(smsc::core::buffers::File& file,IdLtt& data)
{
  data.offset=file.ReadNetInt64();
  data.ltt=file.ReadNetInt32();
  return 12;
}

void XTPrintData(const IdLtt& data)
{
  printf("(%lld:%d)",data.offset,data.ltt);
}

/*
int memsz[256]={
  0,
};
int memszmx[256]={
  0,
};
int memfree[256]={
  0,
};
int memfreemx[256]={
  0,
};
size_t mem=0;
void* operator new(size_t sz)throw(std::bad_alloc)
{
  if(sz<256)
  {
    memsz[sz]++;
    if(memsz[sz]>memszmx[sz])
    {
      memszmx[sz]=memsz[sz];
    }
    if(memfree[sz])
    {
      memfree[sz]--;
    }
  }

  void* ptr=malloc(sz+sizeof(sz));
  *((size_t*)ptr)=sz;
  mem+=sz;
  return ((size_t*)ptr)+1;
}

void* operator new[](size_t sz)throw(std::bad_alloc)
{
  if(sz<256)
  {
    memsz[sz]++;
    if(memsz[sz]>memszmx[sz])
    {
      memszmx[sz]=memsz[sz];
    }
    if(memfree[sz])
    {
      memfree[sz]--;
    }
  }
  void* ptr=malloc(sz+sizeof(sz));
  *((size_t*)ptr)=sz;
  mem+=sz;
  return ((size_t*)ptr)+1;
}
void operator delete(void* ptr)throw()
{
  if(!ptr)return;
  ptr=(void*)(((size_t*)ptr)-1);
  size_t sz=*((size_t*)ptr);
  if(sz<256)
  {
    memsz[sz]--;
    memfree[sz]++;
    if(memfree[sz]>memfreemx[sz])
    {
      memfreemx[sz]=memfree[sz];
    }
  }
  mem-=sz;
  free(ptr);
}
void operator delete[](void* ptr)throw()
{
  ptr=(void*)(((size_t*)ptr)-1);
  size_t sz=*((size_t*)ptr);
  if(sz<256)
  {
    memsz[sz]--;
    memfree[sz]++;
    if(memfree[sz]>memfreemx[sz])
    {
      memfreemx[sz]=memfree[sz];
    }
  }
  mem-=sz;
  free(ptr);
}
*/

int main()
{
#ifndef NOLOGGERPLEASE
  smsc::logger::Logger::Init();
#endif

  /*
  uint8_t* memstart=(uint8_t*)sbrk(0);

  DiskXTree<IdLtt> idIdx,srcSmeIdx,dstSmeIdx,srcAddrIdx,dstAddrIdx,routeIdx;
  int cnt=0;
  for(int i=0;i<1;i++)
  {
    char buf[64];
    sprintf(buf,"%02d.arc",i);
    printf("%s\n",buf);
    sopen(buf);
    SMSId id;
    SMS sms;
    IdLtt v={
      0,0
    };
    while(sload(id,sms))
    {
      sprintf(buf,"%lld",id);
      idIdx.Insert(buf,v);
      srcSmeIdx.Insert(sms.getSourceSmeId(),v);
      dstSmeIdx.Insert(sms.getDestinationSmeId(),v);
      srcAddrIdx.Insert(sms.getOriginatingAddress().toString().c_str(),v);
      dstAddrIdx.Insert(sms.getDestinationAddress().toString().c_str(),v);
      routeIdx.Insert(sms.getRouteId(),v);
      cnt++;
    }
  }
  File fidx;
  fidx.WOpen("smsid.idx");
  idIdx.WriteToFile(fidx);
  fidx.WOpen("srcsme.idx");
  srcSmeIdx.WriteToFile(fidx);
  fidx.WOpen("dstsme.idx");
  dstSmeIdx.WriteToFile(fidx);
  fidx.WOpen("srcaddr.idx");
  srcAddrIdx.WriteToFile(fidx);
  fidx.WOpen("dstaddr.idx");
  dstAddrIdx.WriteToFile(fidx);
  fidx.WOpen("route.idx");
  routeIdx.WriteToFile(fidx);
  printf("%d\n",cnt);
  printf("mem=%lld\n",mem);
  uint8_t* memend=(uint8_t*)sbrk(0);
  printf("memraw=%lld\n",memend-memstart);
  for(int i=0;i<256;i++)
  {
    printf("%d:%d(%d:%d)\n",i,memsz[i],memszmx[i],memfreemx[i]);
  }
  */

  DiskXTree<IdLtt> xt;
  IdLtt v={0,0};
  xt.Insert("hello",v);v.offset++;
  xt.Insert("hello",v);v.offset++;
  xt.Insert("world",v);v.offset++;
  xt.Insert("world",v);v.offset++;
  xt.Insert("world",v);v.offset++;
  xt.Insert("world",v);v.offset++;
  xt.Insert("world",v);v.offset++;
  xt.Insert("world",v);v.offset++;
  xt.Insert("world",v);v.offset++;
  xt.Insert("world",v);v.offset++;
  xt.Insert("world",v);v.offset++;
  xt.Insert("world",v);v.offset++;
  xt.Dump();
  File f;
  f.WOpen("test.bin");
  xt.WriteToFile(f);
  xt.Clear();
  f.ROpen("test.bin");
  std::vector<IdLtt> rv;
  xt.Lookup(f,"world",rv);
  printf("rv.size=%lld\n",(long long int)rv.size());
  f.Seek(0);
  xt.ReadFromFile(f);
  xt.Dump();
}
