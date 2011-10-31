#ifndef _SCAG_PVSS_SERVER_PERSPROTOCOL_H_
#define _SCAG_PVSS_SERVER_PERSPROTOCOL_H_

#include "logger/Logger.h"
#include "scag/util/storage/SerialBuffer.h"
#include "scag/pvss/common/PvapException.h"
#include "scag/pvss/api/packets/Response.h"
#include "scag/pvss/api/packets/ProfileCommandVisitor.h"
#include "scag/pvss/api/packets/ResponseVisitor.h"
#include "scag/pvss/api/packets/ProfileResponseVisitor.h"
#include "scag/pvss/api/packets/BatchRequestComponent.h"
#include "scag/pvss/api/packets/BatchCommand.h"
#include "scag/pvss/data/ProfileKey.h"
#include "scag/pvss/base/Types.h"

namespace scag2 {
namespace pvss  {

class PersProtocol 
{
public:
    PersProtocol():logger_(smsc::logger::Logger::getInstance("persprot")) {};
    void serialize(const Packet& pack,
                   util::storage::SerialBuffer& sb) const; 
    Request* deserialize(util::storage::SerialBuffer& sb) const; 
private:
  class SerialBufferResponseVisitor : public ResponseVisitor, ProfileResponseVisitor {
  public:
      SerialBufferResponseVisitor(util::storage::SerialBuffer& buff):buff_(buff) {};
    virtual bool visitErrResponse(ErrorResponse &resp) /* throw(PvapException) */ ;
    virtual bool visitAuthResponse(AuthResponse &resp) /* throw(PvapException) */ ;
    virtual bool visitProfileResponse(ProfileResponse &resp) /* throw(PvapException) */ ;
    virtual bool visitPingResponse(PingResponse &resp) /* throw(PvapException) */ ;

    virtual bool visitBatchResponse(BatchResponse &resp) /* throw(PvapException) */ ;
    virtual bool visitDelResponse(DelResponse &resp) /* throw(PvapException) */ ;
    virtual bool visitGetResponse(GetResponse &resp) /* throw(PvapException) */ ;
    virtual bool visitIncResponse(IncResponse &resp) /* throw(PvapException) */ ;
    virtual bool visitSetResponse(SetResponse &resp) /* throw(PvapException) */ ;
  private:
    uint8_t getResponseStatus(uint8_t status) const;
  private:
      util::storage::SerialBuffer& buff_;
  };
  
private:
    void deserializeProfileKey(ProfileKey& key, util::storage::SerialBuffer& sb) const;
    BatchRequestComponent* deserializeCommand(PersCmd cmd, util::storage::SerialBuffer& sb) const;
    BatchCommand* deserializeBatchCommand( util::storage::SerialBuffer& sb) const;
    BatchCommand* deserializeOldBatchCommand(uint16_t cmdCount, PersCmd cmdId, const ProfileKey& key, util::storage::SerialBuffer& sb) const;
  bool notSupport(PersCmd cmd) const;
private:
    mutable smsc::logger::Logger* logger_;
};

}
}

#endif

