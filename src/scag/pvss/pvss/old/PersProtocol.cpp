#include "PersProtocol.h"
#include "scag/pvss/common/ScopeType.h"
#include "scag/pvss/api/pvap/Exceptions.h"
#include "scag/pvss/api/packets/Request.h"
#include "scag/pvss/api/packets/ProfileRequest.h"
#include "scag/pvss/api/packets/AbstractProfileRequest.h"
#include "scag/pvss/api/packets/PingRequest.h"
#include "scag/pvss/api/packets/SetCommand.h"
#include "scag/pvss/api/packets/DelCommand.h"
#include "scag/pvss/api/packets/GetCommand.h"
#include "scag/pvss/api/packets/IncCommand.h"
#include "scag/pvss/api/packets/IncModCommand.h"
#include "scag/pvss/api/packets/Response.h"
#include "scag/pvss/api/packets/SetResponse.h"
#include "scag/pvss/api/packets/DelResponse.h"
#include "scag/pvss/api/packets/GetResponse.h"
#include "scag/pvss/api/packets/IncResponse.h"
#include "scag/pvss/api/packets/AuthResponse.h"
#include "scag/pvss/api/packets/BatchResponse.h"
#include "scag/pvss/api/packets/ErrorResponse.h"
#include "scag/pvss/api/packets/PingResponse.h"

namespace scag2 {
namespace pvss  {

  class ProfileRequestCreator: public ProfileCommandVisitor {
  public:
    ProfileRequestCreator(ProfileKey* key):key_(key) {};
    virtual bool visitBatchCommand(BatchCommand &cmd) throw(PvapException) {
      request_.reset( new ProfileRequest<BatchCommand>(&cmd, *key_) );
      return true;
    }
    virtual bool visitDelCommand(DelCommand &cmd) throw(PvapException) {
      request_.reset( new ProfileRequest<DelCommand>(&cmd, *key_) );
      return true;
    }
    virtual bool visitGetCommand(GetCommand &cmd) throw(PvapException) {
      request_.reset( new ProfileRequest<GetCommand>(&cmd, *key_) );
      return true;
    }
    virtual bool visitIncCommand(IncCommand &cmd) throw(PvapException) {
      request_.reset( new ProfileRequest<IncCommand>(&cmd, *key_) );
      return true;
    }
    virtual bool visitIncModCommand(IncModCommand &cmd) throw(PvapException) {
      request_.reset( new ProfileRequest<IncModCommand>(&cmd, *key_) );
      return true;
    }
    virtual bool visitSetCommand(SetCommand &cmd) throw(PvapException) {
      request_.reset( new ProfileRequest<SetCommand>(&cmd, *key_) );
      return true;
    }
    AbstractProfileRequest * getRequest() {
      return request_.release();
    }
  private:
    std::auto_ptr<AbstractProfileRequest> request_;
    ProfileKey *key_;
  };


bool PersProtocol::notSupport(PersCmd cmd) const {
  return (cmd > PC_MTBATCH || cmd == PC_BATCH
          || cmd == PC_TRANSACT_BATCH || cmd == PC_UNKNOWN) ? true : false;
}

Request* PersProtocol::deserialize(SerialBuffer& sb) const {
  smsc_log_debug(logger_, "deserialize request");
  PersCmd cmd = static_cast<PersCmd>(sb.ReadInt8());
  smsc_log_debug(logger_, "Command %d received", cmd);
  uint32_t seqNum = -1;
  if (notSupport(cmd)) {
    smsc_log_debug(logger_, "Command %d not supports", cmd);
    throw pvap::InvalidMessageTypeException(seqNum, static_cast<int>(cmd));
  }
  if (cmd == PC_PING) {
    smsc_log_debug(logger_, "Ping received");
    return new PingRequest();
  }

  ProfileKey key;
  deserializeProfileKey(key, sb);

  ProfileRequestCreator requestCreator(&key);
  AbstractCommand* request = 0;
  if (cmd == PC_MTBATCH) {
    smsc_log_debug(logger_, "Batch received");
    request = deserializeBatchCommand(sb);
  } else {
    request = deserializeCommand(cmd, sb);
  }
  request->visit(requestCreator);
  return requestCreator.getRequest();
}

void PersProtocol::deserializeProfileKey(ProfileKey& key, SerialBuffer& sb) const {
  ScopeType scopeType = static_cast<ScopeType>(sb.ReadInt8());
  switch (scopeType) {
  case ABONENT: {
    std::string strKey;
    sb.ReadString(strKey);
    key.setAbonentKey(strKey);    
  }
  case OPERATOR: key.setOperatorKey(sb.ReadInt32());
  case PROVIDER: key.setProviderKey(sb.ReadInt32());
  case SERVICE:  key.setServiceKey(sb.ReadInt32());
  }
}

/*
void PersPvapProtocol::deserialize(SingleRequest* request, SerialBuffer& sb) const {
  deserializeHeader(request, sb);
  request->setCommand(deserializeCommand(request->getCmdId(), sb));
}
*/
BatchCommand* PersProtocol::deserializeBatchCommand(SerialBuffer& sb) const {
  uint16_t cmdCount = sb.ReadInt16();
  std::auto_ptr<BatchCommand> batch( new BatchCommand() );
  batch->setTransactional((bool)sb.ReadInt8());
  for (int i = 0; i < cmdCount; ++i) {
    PersCmd cmd = static_cast<PersCmd>(sb.ReadInt8());
    batch->addComponent(deserializeCommand(cmd, sb));
  }
  return batch.release();
}

BatchRequestComponent* PersProtocol::deserializeCommand(PersCmd cmdType, SerialBuffer& sb) const {
  switch(cmdType) {
  case PC_DEL: {
    std::string varName;
    sb.ReadString(varName);
    DelCommand *cmd = new DelCommand();
    cmd->setVarName(varName);
    return cmd;
  }
  case PC_SET: {
    std::auto_ptr<Property> prop(new Property);
    prop->Deserialize(sb);
    SetCommand *cmd = new SetCommand();
    cmd->setProperty(prop.release());
    return cmd;
  }
  case PC_GET: {
    std::string varName;
    sb.ReadString(varName);
    GetCommand *cmd = new GetCommand();
    cmd->setVarName(varName);
    return cmd;
  }
  case PC_INC:
  case PC_INC_RESULT: {
    std::auto_ptr<Property> prop(new Property);
    prop->Deserialize(sb);
    IncCommand *cmd = new IncCommand();
    cmd->setProperty(prop.release());
    return cmd;
  }
  case PC_INC_MOD: {
    uint32_t mod = sb.ReadInt32();
    std::auto_ptr<Property> prop(new Property);
    prop->Deserialize(sb);
    IncModCommand *cmd = new IncModCommand();
    cmd->setModulus(mod);
    cmd->setProperty(prop.release());
    return cmd;
  }
  default: throw pvap::InvalidMessageTypeException(-1, cmdType);
  }
}

void PersProtocol::serialize(const Packet& pack, SerialBuffer& sb) const {
  SerialBufferResponseVisitor visitor(sb);
  Packet& packet = const_cast<Packet&>(pack);
  if ( packet.isRequest() ) {
    return;
  }
  static_cast< Response& >(packet).visit( visitor );
}

bool PersProtocol::SerialBufferResponseVisitor::visitAuthResponse(AuthResponse &resp) throw(PvapException) {
  return false;
}

bool PersProtocol::SerialBufferResponseVisitor::visitBatchResponse(BatchResponse &resp) throw(PvapException) {
  SerialBufferResponseVisitor visitor(buff_);
  std::vector<BatchResponseComponent*> content = resp.getBatchContent();
  std::vector<BatchResponseComponent*>::iterator i = content.begin();
  for (; i != content.end(); ++i) {
    (*i)->visit( visitor );
  }
  return true;
}

bool PersProtocol::SerialBufferResponseVisitor::visitDelResponse(DelResponse &resp) throw(PvapException) {
  buff_.WriteInt8(resp.getStatus());
  return true;
}

bool PersProtocol::SerialBufferResponseVisitor::visitErrResponse(ErrorResponse &resp) throw(PvapException) {
  buff_.WriteInt8(resp.getStatus());
  return true;
}

bool PersProtocol::SerialBufferResponseVisitor::visitGetResponse(GetResponse &resp) throw(PvapException) {
  buff_.WriteInt8(resp.getStatus());
  resp.getProperty()->Serialize(buff_);
  return true;
}

bool PersProtocol::SerialBufferResponseVisitor::visitIncResponse(IncResponse &resp) throw(PvapException) {
  buff_.WriteInt8(resp.getStatus());
  buff_.WriteInt32(resp.getResult());
  return true;
}

bool PersProtocol::SerialBufferResponseVisitor::visitPingResponse(PingResponse &resp) throw(PvapException) {
  buff_.WriteInt8(resp.getStatus());
  return true;
}

bool PersProtocol::SerialBufferResponseVisitor::visitSetResponse(SetResponse &resp) throw(PvapException) {
  buff_.WriteInt8(resp.getStatus());
  return true;
}

/*
void PersPvapProtocol::serialize(const ResponsePacket& packet, SerialBuffer& sb) const {
  uint32_t seqNumber = packet.getSequenceNumber();
  if (seqNumber > 0) {
    sb.WriteInt32(seqNumber);
  }
  ResponsePacket::Response resp = packet.getResponse();
  sb.WriteInt8(resp);
  if (resp != RESPONSE_OK) {
    return;
  }
  PersCmd cmd = packet.getCmdId();
  if (cmd == PC_SET || cmd == PC_DEL) {
    return;
  }
  if (cmd == PC_GET) {
    packet.getProperty().Serialize(sb);
    return;
  }
  sb.WriteInt32(packet.getResult());
}

void PersPvapProtocol::serialize(const RequestPacket* packet, SerialBuffer& sb) const {
  const ResponsePacket& fakeResp = packet->getFakeResponse();
  if (fakeResp.getResponse() != RESPONSE_OK) {
    serialize(fakeResp, sb);
    return;
  }
  if (!packet->isBatch()) {
    serialize(packet->getResponse(), sb);
    return;
  }
  const BatchRequest* batch = 0;
  try {
    batch = dynamic_cast<BatchRequest*>(const_cast<RequestPacket*>(packet));
    require(batch);
  } catch (const std::bad_cast& ex) {
    smsc_log_error(logger_, "PersProtocol::serialize: bad cast exception: %s", ex.what());
    abort();
  }
  uint16_t cmdCount = batch->getCommandsCount();
  if (!batch->isTransact()) {
    for (uint16_t i = 0; i < cmdCount; ++i) {
      serialize(batch->getResponse(i), sb);
    }
    return;
  }
  for (uint16_t i = 0; i < cmdCount; ++i) {
    const ResponsePacket& resp = batch->getResponse(i);
    serialize(resp, sb);
    if (resp.getResponse() != RESPONSE_OK) {
      return;
    }
  }

}
*/

/*
void PersProtocol::serialize(const GetResponse* packet, SerialBuffer& sb) const {
  sb.WriteInt8(packet->getResponse());
  packet->getProperty().serialize(sb);
}

void PersProtocol::serialize(const IncResponse* packet, SerialBuffer& sb) const {
  sb.WriteInt8(packet->getResponse());
  sb.WriteInt32(packet->getResult());
}
*/
}
}


