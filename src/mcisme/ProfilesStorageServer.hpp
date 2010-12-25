//------------------------------------
//  ProfilesStorageServer.hpp
//  Routman Michael, 2005-2006
//------------------------------------


#ifndef ___PROFILESSTORAGESERVER_H
#define ___PROFILESSTORAGESERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include <logger/Logger.h>

#include <core/buffers/DiskHash.hpp>

#include <mcisme/AbntAddr.hpp>
#include <mcisme/Profiler.h>
#include <mcisme/ProfilesStorage.hpp>
#include <core/buffers/File.hpp>
#include <admin/util/SocketListener.h>

namespace smsc {
namespace mcisme {

using smsc::core::threads::ThreadedTask;
using smsc::admin::util::SocketListener;

const uint8_t MSG_ID_PROFILE = 0;
const uint8_t MSG_ID_GETPROF = 1;
const uint8_t MSG_ID_SETPROF = 2;

const uint8_t STATUS_OK         = 0;
const uint8_t STATUS_ERRROR     = 1;
const uint8_t STATUS_BADMSG     = 2;
const uint8_t STATUS_NO_ABNT    = 3;

struct ProfileHeader
{
  uint8_t       msgId;
  uint8_t       SeqNum;
  uint8_t       Status;
};

struct ProfileBody
{
  char  abnt[32];
  uint8_t       eventMask;
  uint8_t       inform;
  uint8_t       notify;
  uint8_t       wantNotifyMe;
  int8_t        informTemplateId;
  int8_t        notifyTemplateId;
};

struct ProfileMessage
{
  ProfileHeader hdr;
  ProfileBody   body;
};

class ProfStorageCommandDispatcher: public ThreadedTask 
{
public:
  ProfStorageCommandDispatcher(core::network::Socket * admSocket)
    : logger(smsc::logger::Logger::getInstance("mcisme.prof")),
      task_name("ProfStorageCmdDisp"), pProfStorage(0), timeout(15)
  {
    sock = admSocket;
    smsc_log_debug(logger, "Command dispatcher created.");
  }

  virtual ~ProfStorageCommandDispatcher()
  {
    smsc_log_debug(logger, "Command dispatcher \"%s\" destroyed.", task_name);
    if (sock != 0)
    {
      sock->Abort();
      delete sock;
      sock = 0;
    }
  }

  virtual int Execute(void)
  {
    smsc_log_info(logger, "Command dispather started");

    pProfStorage = ProfilesStorage::GetInstance();
    if(pProfStorage)
      smsc_log_info(logger, "ProfileServer: Getting profile storage instance Ok");
    else
    {
      smsc_log_info(logger, "ProfileServer: getting profile storage instance failed. Stopping CommandDispatcher");
      isStopping = 1;
    }

    while(!isStopping)
    {
      switch(sock->canRead())
      {
      case 1:	
        if(!Process())
        {
          smsc_log_info(logger, "ProfileServer: Process incoming data failed. Disconnecting.");
          isStopping = 1;
        }
        break;
      case 0:	break;
      case -1:
        smsc_log_info(logger, "ProfileServer: Socket disconnected");
        isStopping = 1;
        break;
      default:
        smsc_log_info(logger, "ProfileServer: Unknown error on canRead()");
        isStopping = 1;
        break;
      }
    }
    sock->Abort();
    delete sock;
    sock = 0;
    smsc_log_info(logger, "Command dispather stopped");
    return 0;
  }

  virtual const char* taskName(){return task_name;}
  static void shutdown(){}

protected:

  bool Process(void)
  {
    smsc_log_info(logger, "ProfileServer: Process incomming data");

    ProfileMessage msg;

    if(!recvnbytes((char*)&msg, sizeof(ProfileMessage)))
    {
      smsc_log_info(logger, "ProfileServer: recvnbytes failed.");
      return false;
    }

    //if(!RecvMsg(&msg))
    //{
    //	smsc_log_info(logger, "ProfileServer: RecvMsg failed.");
    //	return false;
    //}
    smsc_log_info(logger, "Recieved:--------------------------------");
    smsc_log_info(logger, "msg.hdr.msgId = %d", msg.hdr.msgId);
    smsc_log_info(logger, "msg.hdr.SeqNum = %d", msg.hdr.SeqNum);
    smsc_log_info(logger, "msg.hdr.Status = %d", msg.hdr.Status);

    smsc_log_info(logger, "msg.body.abnt = %s", msg.body.abnt);
    smsc_log_info(logger, "msg.body.eventMask = %d", msg.body.eventMask);
    smsc_log_info(logger, "msg.body.inform = %d", msg.body.inform);
    smsc_log_info(logger, "msg.body.informTemplateId = %d", msg.body.informTemplateId);
    smsc_log_info(logger, "msg.body.notify = %d", msg.body.notify);
    smsc_log_info(logger, "msg.body.notifyTemplateId = %d", msg.body.notifyTemplateId);
    smsc_log_info(logger, "msg.body.wantNotifyMe = %d", msg.body.wantNotifyMe);
    smsc_log_info(logger, "------------------------------------------");

    if(msg.hdr.msgId == MSG_ID_GETPROF)			// Getting profile
    {
      smsc_log_info(logger, "ProfileServer: Recieved MSG_GETPROF.");
      AbntAddr		abnt(msg.body.abnt);
      AbonentProfile	prof;

      bool ret = pProfStorage->Get(abnt, prof);
      msg.hdr.msgId = MSG_ID_PROFILE;
      msg.hdr.Status = ret?STATUS_OK:STATUS_NO_ABNT;
      msg.body.eventMask = prof.eventMask;
      msg.body.inform = prof.inform;
      msg.body.informTemplateId = prof.informTemplateId;
      msg.body.notify = prof.notify;
      msg.body.notifyTemplateId = prof.notifyTemplateId;
      msg.body.wantNotifyMe = prof.wantNotifyMe;

      smsc_log_info(logger, "Get: msg.hdr.msgId = %d,msg.hdr.SeqNum = %d,msg.hdr.Status = %d,msg.body.abnt = %s,msg.body.eventMask = %d,"
                    "msg.body.inform = %d,msg.body.informTemplateId = %d,msg.body.notify = %d,msg.body.notifyTemplateId = %d,msg.body.wantNotifyMe = %d",
                    msg.hdr.msgId, msg.hdr.SeqNum, msg.hdr.Status, msg.body.abnt, msg.body.eventMask, msg.body.inform,
                    msg.body.informTemplateId, msg.body.notify, msg.body.notifyTemplateId, msg.body.wantNotifyMe);

      sendnbytes((char*)&msg, sizeof(ProfileMessage));
    }
    else if(msg.hdr.msgId == MSG_ID_SETPROF)	// Setting profile
    {
      smsc_log_info(logger, "ProfileServer: Recieved MSG_SETPROF.");
      AbntAddr		abnt(msg.body.abnt);
      AbonentProfile	prof;

      prof.eventMask = msg.body.eventMask;
      prof.inform = msg.body.inform;
      prof.informTemplateId = msg.body.informTemplateId;
      prof.notify = msg.body.notify;
      prof.notifyTemplateId = msg.body.notifyTemplateId;
      prof.wantNotifyMe = msg.body.wantNotifyMe;

      pProfStorage->Set(abnt, prof);
      msg.hdr.msgId = MSG_ID_PROFILE;
      msg.hdr.Status = STATUS_OK;
      sendnbytes((char*)&msg, sizeof(ProfileMessage));
    }
    else										// Unknown command
    {
      smsc_log_info(logger, "ProfileServer: Unknown command was recieved from client.");
      return false;
    }
    return true;
  }
  bool RecvMsg(ProfileMessage* msg)
  {
    if(!recvnbytes((char*)msg, sizeof(ProfileHeader)))
    {
      smsc_log_info(logger, "ProfileServer: RecvMsg error: recvnbytes failed while reading a ProfileHeader");
      return false;
    }
    if(msg->hdr.msgId == MSG_ID_GETPROF)
      return true;
		
    if(msg->hdr.msgId == MSG_ID_SETPROF)
      if(!recvnbytes((char*)&(msg->body.eventMask), sizeof(ProfileBody)))
      {
        smsc_log_info(logger, "ProfileServer: RecvMsg error: recvnbytes failed while reading a ProfileBody");
        return false;
      }
    return true;
  }
  bool recvnbytes(char* msg, size_t n)
  {
    size_t len, ret;
    //		smsc_log_info(logger, "ProfileServer: recvnbytes n = %d", n);
    ret = sock->Read(msg, static_cast<int>(n));
    if(-1 == ret || 0 == ret)
    {
      smsc_log_info(logger, "ProfileServer: recvnbytes error: Read return %d", ret);
      return false; 
    }
    len = ret;
    while(len < n)
    {
      if(1 != (ret = sock->canRead(timeout)))
      {
        smsc_log_info(logger, "ProfileServer: recvnbytes error: canRead return %d", ret);
        return false;
      }
      ret = sock->Read(msg+len, static_cast<int>(n-len));
      if(-1 == ret || 0 == ret)
      {
        smsc_log_info(logger, "ProfileServer: recvnbytes error: Read return %d", ret);
        return false; 
      }
      len += ret;
    }
    return true;
  }
  bool sendnbytes(char* msg, size_t n)
  {
    int len=0, ret;

    while(len < n)
    {
      if(1 != (ret = sock->canWrite(timeout)))
      {
        smsc_log_info(logger, "ProfileServer: sendnbytes error: canWrite return %d", ret);
        return false;
      }
      ret = sock->Write(msg+len, static_cast<int>(n-len));
      if(-1 == ret || 0 == ret)
      {
        smsc_log_info(logger, "ProfileServer: sendnbytes error: Write return %d", ret);
        return false; 
      }
      len += ret;
    }
    return true;
  }

  core::network::Socket * sock;
  smsc::logger::Logger *logger;
  const char * const task_name;
  ProfilesStorage* pProfStorage;
  int	timeout;
};

typedef SocketListener<ProfStorageCommandDispatcher> ProfilesStorageServer;

};	//  namespace msisme
};	//  namespace smsc
#endif
