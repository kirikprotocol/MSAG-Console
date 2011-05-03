#ifndef __SMSC_ADMIN_HSDAEMON_INTERCONNECTCOMMANDS_H__
#define __SMSC_ADMIN_HSDAEMON_INTERCONNECTCOMMANDS_H__

#include "core/network/Socket.hpp"
#include "Service.h"
#include "core/buffers/TmpBuf.hpp"
#include <string>
#include <list>

namespace smsc{
namespace admin{
namespace hsdaemon{

namespace interconnect{

namespace net=smsc::core::network;
namespace buf=smsc::core::buffers;

inline uint16_t readUInt16(net::Socket* sck)
{
  uint16_t rv;
  if(sck->ReadAll(reinterpret_cast<char*>(&rv),sizeof(rv),60)==-1)throw Exception("Failed to read uint16_t from socket");
  return ntohs(rv);
}

inline uint32_t readUInt32(net::Socket* sck)
{
  uint32_t rv;
  if(sck->ReadAll(reinterpret_cast<char*>(&rv),sizeof(rv),60)==-1)throw Exception("Failed to read uint32_t from socket");
  return ntohl(rv);
}

inline std::string readString(net::Socket* sck)
{
  uint16_t len=readUInt16(sck);
  buf::TmpBuf<char,64> buf(len);
  if(sck->ReadAll(buf.get(),len,60)==-1)throw Exception("Failed to read string of length %d from socket",len);
  std::string rv;
  return std::string(buf.get(),len);
}

inline void writeUInt16(net::Socket* sck,uint16_t val)
{
  val=htons(val);
  if(sck->WriteAll(reinterpret_cast<char*>(&val),sizeof(val))==-1)throw Exception("Failed to write uint16_t to socket");
}

inline void writeUInt32(net::Socket* sck,uint32_t val)
{
  val=htonl(val);
  if(sck->WriteAll(reinterpret_cast<char*>(&val),sizeof(val))==-1)throw Exception("Failed to write uint32_t to socket");
}

inline void writeString(net::Socket* sck,const std::string& val)
{
  uint16_t len=static_cast<uint16_t>(val.length());
  writeUInt16(sck,len);
  if(sck->WriteAll(val.c_str(),len)==-1)throw Exception("Failed to write string of length %d to socket",len);
}

enum Commands{
  cmdSynchronizeServices=1,
  cmdGetServiceStatus,
  cmdStartService,
  cmdShutdownService,
  cmdListServices,
  cmdGetServiceProperty,
  cmdAddServiceFromConfig
};

enum ServiceProperty{
  propArgs=1,
  propPreferredNode,
  propLogicalHostname,
  propServiceType,
  propAutostartDelay
};


inline Service::run_status remoteGetServiceStatus(net::Socket& clntSck,const char* svc)
{
  writeUInt16(&clntSck,cmdGetServiceStatus);
  writeString(&clntSck,svc);
  uint16_t rv=readUInt16(&clntSck);
  clntSck.Abort();
  return (Service::run_status)rv;
}

inline bool remoteStartService(net::Socket& clntSck,const char* svc)
{
  writeUInt16(&clntSck,cmdStartService);
  writeString(&clntSck,svc);
  uint16_t rv=readUInt16(&clntSck);
  clntSck.Abort();
  return rv!=0;
}

inline bool remoteShutdownService(net::Socket& clntSck,const char* svc,bool switchOver)
{
  writeUInt16(&clntSck,cmdShutdownService);
  writeString(&clntSck,svc);
  writeUInt16(&clntSck,switchOver?1:0);
  uint16_t rv=readUInt16(&clntSck);
  clntSck.Abort();
  return rv!=0;
}

inline void remoteListServices(net::Socket& clntSck,std::list<std::string>& lst)
{
  writeUInt16(&clntSck,cmdListServices);
  int cnt=readUInt16(&clntSck);
  for(int i=0;i<cnt;i++)
  {
    lst.push_back(readString(&clntSck));
  }
  clntSck.Abort();
}

std::string remoteGetServiceProperty(net::Socket& clntSck,const char* svc,ServiceProperty prop)
{
  writeUInt16(&clntSck,cmdGetServiceProperty);
  writeString(&clntSck,svc);
  writeUInt16(&clntSck,prop);
  return readString(&clntSck);
}

std::string addServiceFromConfig(net::Socket& clntSck,const char* svc)
{
  writeUInt16(&clntSck,cmdAddServiceFromConfig);
  writeString(&clntSck,svc);
  return readString(&clntSck);
}

}//interconnect

}//hsdaemon
}//admin
}//smsc

#endif
