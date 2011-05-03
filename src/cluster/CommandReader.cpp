#include "CommandReader.h"
#include "core/network/Socket.hpp"
#include <memory.h>

namespace smsc {
namespace cluster {

using smsc::core::network::Socket;
using namespace smsc::util;

void CommandReader::Start()
{
  stop = false;
  Thread::Start();
}

int CommandReader::Execute()
{
    smsc_log_info(logger, "Reader started");
  while(!stop)
  {
    //printf("reader, Execute, wait for command...\n");
    if(clntSock)delete clntSock;
    clntSock=srvSock->Accept();
    if ( clntSock)
    {
       smsc_log_info(logger, "Reader accepted connection");
       for( ;; )
       {
         try{
           smsc_log_info(logger, "Reader reading command");
           Command *cmd = readCommand(clntSock);
           if(cmd)
           {
             smsc_log_info(logger, "Command %02X read", cmd->getType());
             if(cmd->getType() == GETROLE_CMD)
             {
               writeRole(clntSock, cmd);
             }
             else
             {
               if(*role == SLAVE)
               {
                 dispatcher->addCommand(cmd);
               }
               else
               {
                 smsc_log_info(logger,"Command skipped, role!=SLAVE");
                 if(cmd)delete cmd;
               }
             }
           }else
           {
             smsc_log_warn(logger,"Command deserialization failed");
           }
         }catch(Exception & e)
         {
             smsc_log_info(logger, "Reader exception, %s", e.what());
             break;
         }catch(...){
             smsc_log_info(logger, "Reader exception, unexpected error");
             break;
         }

       }
    }
    //printf("reader, Execute, command is accepted\n");

  }
  smsc_log_info(logger, "Reader is stoped");

  return 0;
}

void CommandReader::readHeader(Socket * socket, uint32_t &type, uint32_t &len)
{
  uint8_t buffer[8];

  read(socket, buffer, 8);

  uint32_t val;
  memcpy((void*)&val,     (const void*)buffer, 4);
  type = ntohl(val);

  memcpy((void*)&val,   (const void*)(buffer + 4), 4);
  len = ntohl(val);
}

Command* CommandReader::readCommand(Socket * socket)
{
    uint32_t type;
    uint32_t len;

    readHeader(socket, type, len);

    smsc_log_info(logger, "readCommand, Command header: type: %02X, len: %d", type, len);

    if(type == GETROLE_CMD)
    {
      Command *cmd = new GetRoleCommand( (int)( *role ));
      return cmd;
    }

    if(len>0)
    {
      std::auto_ptr<uint8_t> buffer ( new uint8_t[len] );

      read(socket, (void*)buffer.get(), len);

      return Command::create((CommandType)type, (void*)buffer.get(), len);
    }else
    {
      return Command::create((CommandType)type, 0,0);
    }
}

void CommandReader::read(Socket * socket, void* buffer, int size)
{
  if(!socket) throw Exception("Command read failed. Socket pointer is NULL.");
  int toRead = size;
  char* readBuffer = (char *)buffer;
  while (toRead > 0)
  {
    int read = socket->canRead(60);
    if (read == 0)
    {
      continue;
    }
    else
      if (read > 0)
      {
        read = socket->Read(readBuffer, toRead);
        if (read > 0)
        {
          readBuffer+=read;
          toRead-=read;
          continue;
        }
      }
    throw Exception("Command read failed. Socket closed. %s", strerror(errno));
  }
}

void CommandReader::writeRole(Socket* socket, Command *cmd)
{
    if(!cmd)
        return;

    uint32_t len = 0;
    std::auto_ptr<uint8_t> buff ( (uint8_t*)cmd->serialize(len) );
    int size = len + 8;
    std::auto_ptr<uint8_t> buffer( new uint8_t[size] );
    uint32_t type = htonl( cmd->getType() );
    memcpy((void*)buffer.get(), (const void*)&type, 4);
    uint32_t val32 = htonl(len);
    memcpy((void*)( buffer.get() + 4), (const void*)&val32, 4);
    memcpy((void*)( buffer.get() + 8), (const void*)buff.get(), len);
    delete cmd;

    if (!socket)
    {
       throw Exception("Role send failed. Socket NULL!");
    }
    int toWrite = size;
    const char* writeBuffer = (const char *)buffer.get();
    int written;
    while (toWrite > 0)
    {
        if (socket->canWrite(10)<=0) throw Exception("Role send failed. Blocked socket.");

        written = socket->Write(writeBuffer, toWrite);
        if (written > 0)
        {
          writeBuffer+=written;
          toWrite-=written;
        }else
        {
          throw Exception("Role send failed. Socket closed. %s", strerror(errno));
        }
    }
}

}
}
