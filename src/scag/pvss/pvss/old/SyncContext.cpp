#include <exception>
#include "SyncContext.h"
#include "scag/util/RelockMutexGuard.h"
#include "scag/pvss/api/core/Core.h"
#include "scag/pvss/api/pvap/Exceptions.h"
#include "scag/pvss/common/PvapException.h"
#include "scag/pvss/api/packets/Response.h"
#include "scag/pvss/api/packets/ErrorResponse.h"
#include "scag/pvss/api/packets/PingResponse.h"

#include "PersServerContext.h"


namespace scag2 {
namespace pvss  {

using scag::util::RelockMutexGuard;

SyncContext::SyncContext(Socket* sock, WriterTaskManager& writerManager, core::server::ServerCore& server, PersProtocol& protocol, bool perfCounterOn)
             :ConnectionContext(sock, writerManager, server, perfCounterOn), protocol_(protocol), seqNum_(0) 
{
  if (socket_) {
    SocketData::setContext(socket_, this);
  }
  logger_ = Logger::getInstance("synccontext");
  smsc_log_info(logger_, "Connection accepted from %s", peerIp_.c_str());
}

void SyncContext::createErrorResponse(Response::StatusType status) {
  smsc_log_debug(logger_, "%p: Create Error response %d", this, status);
  ErrorResponse resp;
  resp.setStatus(status);
  sendResponse(resp);
}

bool SyncContext::sendResponse(const Response& resp) {
  try {
    smsc_log_debug(logger_, "%p: Serialize response %s", this, resp.toString());
    fakeResp_.Empty();
    protocol_.serialize(resp, fakeResp_);
    return sendResponseData(fakeResp_.c_ptr(), fakeResp_.GetSize());
  } catch (const SerialBufferOutOfBounds& e) {
    smsc_log_warn(logger_, "%p:SerialBufferOutOfBounds while serializing response %s", this, resp.toString().c_str());
  } catch (const PvapException& e) {
    smsc_log_warn(logger_, "%p:PvapException: %s, while serializing response %s", this, e.getMessage().c_str(), resp.toString().c_str());
  } catch (const std::exception& e) {
    smsc_log_warn(logger_, "%p:std::exception %s, while serializing response %s", this, e.what(), resp.toString().c_str());
  } catch (...) {
    smsc_log_warn(logger_, "%p:Unknown Exception, while serializing response %s", this, resp.toString().c_str());
  }
  return false;
}

void SyncContext::sendResponse(const Response* resp) {
  if (!resp) {
    smsc_log_debug(logger_, "SyncContext::sendResponse: context:%p response is NULL", this);
    return;
  }
  core::Core::PacketState state = sendResponse(*resp) ? core::Core::SENT : core::Core::FAILED;
  pvssServer_.reportPacket(resp->getSeqNum(), *getSocket(), state);
}

bool SyncContext::processReadSocket(const time_t& now) {
  if (!async_) {
    RelockMutexGuard mg(mutex_);
    if (action_ != READ_REQUEST) {
      mg.Unlock();
      smsc_log_warn(logger_, "cx:%p socket %p error action=%d, must be READ_REQUEST", this, socket_, action_);
      return false;
    }
  }

  if (!readData(now)) {
    return false;
  }

  if (inbuf_.GetSize() < PACKET_LENGTH_SIZE || inbuf_.GetSize() < packetLen_) {
    return true;
  }

  try {
  
    Request *req = protocol_.deserialize(inbuf_);
    if (!req) {
      smsc_log_warn(logger_, "%p:request is NULL. buffer received len=%d curpos=%d data=%s",
                              this, inbuf_.GetSize(), inbuf_.GetPos(), inbuf_.toString().c_str());
      createErrorResponse(Response::ERROR);
    }
  
    std::auto_ptr<Request> request(req);
    inbuf_.Empty();
    packetLen_ = 0;
  
    if (request->isPing()) {
      PingResponse resp;
      resp.setStatus(Response::OK);
      sendResponse(resp);
      return true;
    }

    request->setSeqNum(seqNum_++);
    std::auto_ptr<core::server::ServerContext> serverContext(new PersServerContext(request.release(), *this));
    pvssServer_.receiveOldPacket(serverContext);

  } catch (const SerialBufferOutOfBounds& e) {
    smsc_log_warn(logger_, "%p:SerialBufferOutOfBounds: bad data in buffer received len=%d curpos=%d data=%s",
                            this, inbuf_.GetSize(), inbuf_.GetPos(), inbuf_.toString().c_str());
    createErrorResponse(Response::BAD_REQUEST);

  } catch (const std::runtime_error& e) {
    smsc_log_warn(logger_, "%p:std::runtime_error: Error profile key: %s. buffer received len=%d curpos=%d data=%s",
                            this, e.what(), inbuf_.GetSize(), inbuf_.GetPos(), inbuf_.toString().c_str());
    createErrorResponse(Response::ERROR);

  } catch (const pvap::InvalidMessageTypeException& e) {
    smsc_log_warn(logger_, "%p:InvalidMessageTypeException: %s. buffer received len=%d curpos=%d data=%s",
                            this, e.getMessage().c_str(), inbuf_.GetSize(), inbuf_.GetPos(), inbuf_.toString().c_str());
    createErrorResponse(Response::NOT_SUPPORTED);

  } catch (const PvapException& e) {
    smsc_log_warn(logger_, "%p:PvapException: %s. buffer received len=%d curpos=%d data=%s",
                            this, e.getMessage().c_str(), inbuf_.GetSize(), inbuf_.GetPos(), inbuf_.toString().c_str());
    createErrorResponse(Response::ERROR);

  } catch (const std::exception& e) {
    smsc_log_warn(logger_, "%p:std::exception: %s. buffer received len=%d curpos=%d data=%s",
                            this, e.what(), inbuf_.GetSize(), inbuf_.GetPos(), inbuf_.toString().c_str());
    createErrorResponse(Response::ERROR);

  } catch (...) {
    smsc_log_warn(logger_, "%p:Unknown Exception: buffer received len=%d curpos=%d data=%s",
                            this, inbuf_.GetSize(), inbuf_.GetPos(), inbuf_.toString().c_str());
    createErrorResponse(Response::ERROR);
  }

  return true;
}



}//pvss
}//scag2

