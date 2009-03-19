#ifndef _SCAG_PVSS_SERVER_PERSPROTOCOL_H_
#define _SCAG_PVSS_SERVER_PERSPROTOCOL_H_

#include "logger/Logger.h"
#include "scag/util/storage/SerialBuffer.h"
#include "scag/pvss/common/PvapException.h"
#include "scag/pvss/api/packets/Packet.h"
#include "scag/pvss/api/packets/Response.h"
#include "scag/pvss/api/packets/ProfileCommandVisitor.h"
#include "scag/pvss/api/packets/ResponseVisitor.h"
#include "scag/pvss/api/packets/BatchRequestComponent.h"
#include "scag/pvss/api/packets/BatchCommand.h"
#include "scag/pvss/data/ProfileKey.h"
#include "scag/pvss/base/Types.h"

namespace scag2 {
namespace pvss  {

using scag::util::storage::SerialBuffer;
using scag::util::storage::SerialBufferOutOfBounds;
using smsc::logger::Logger;

class Request;

class PersProtocol {
public:
  PersProtocol():logger_(Logger::getInstance("persprot")) {};
  void serialize(const Packet& pack, SerialBuffer& sb) const; 
  Request* deserialize(SerialBuffer& sb) const; 
private:
  class SerialBufferResponseVisitor : public ResponseVisitor {
  public:
    SerialBufferResponseVisitor(SerialBuffer& buff):buff_(buff) {};
    virtual bool visitAuthResponse(AuthResponse &resp) throw(PvapException);
    virtual bool visitBatchResponse(BatchResponse &resp) throw(PvapException);
    virtual bool visitDelResponse(DelResponse &resp) throw(PvapException);
    virtual bool visitErrResponse(ErrorResponse &resp) throw(PvapException);
    virtual bool visitGetResponse(GetResponse &resp) throw(PvapException);
    virtual bool visitIncResponse(IncResponse &resp) throw(PvapException);
    virtual bool visitPingResponse(PingResponse &resp) throw(PvapException);
    virtual bool visitSetResponse(SetResponse &resp) throw(PvapException);
  private:
    uint8_t getResponseStatus(Response::StatusType status) const;
  private:
    SerialBuffer& buff_;
  };
  
private:
  void deserializeProfileKey(ProfileKey& key, SerialBuffer& sb) const;
  BatchRequestComponent* deserializeCommand(PersCmd cmd, SerialBuffer& sb) const;
  BatchCommand* deserializeBatchCommand(SerialBuffer& sb) const;
  bool notSupport(PersCmd cmd) const;
private:
  mutable Logger* logger_;
};

}
}

#endif

