#include "PersProtocol.h"
#include "scag/pvss/common/ScopeType.h"
#include "scag/pvss/base/PersServerResponse.h"
#include "scag/pvss/api/pvap/Exceptions.h"
#include "scag/pvss/api/packets/ProfileRequest.h"
#include "scag/pvss/api/packets/ProfileResponse.h"
#include "scag/pvss/api/packets/PingRequest.h"
#include "scag/pvss/api/packets/SetCommand.h"
#include "scag/pvss/api/packets/DelCommand.h"
#include "scag/pvss/api/packets/GetCommand.h"
#include "scag/pvss/api/packets/IncCommand.h"
#include "scag/pvss/api/packets/IncModCommand.h"
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

  // ProfileRequestCreator requestCreator(&key);
  ProfileCommand* request = 0;
  if (cmd == PC_MTBATCH) {
    smsc_log_debug(logger_, "Batch received");
    request = deserializeBatchCommand(sb);
  } else {
    request = deserializeCommand(cmd, sb);
  }
  return request ? new ProfileRequest(key,request) : 0;
}

void PersProtocol::deserializeProfileKey(ProfileKey& key, SerialBuffer& sb) const {
  smsc_log_debug(logger_, "deserialize profile key");
  ScopeType scopeType = static_cast<ScopeType>(sb.ReadInt8());
  switch (scopeType) {
  case ABONENT: {
    std::string strKey;
    sb.ReadString(strKey);
    key.setAbonentKey(strKey);    
    smsc_log_debug(logger_, "abonent key='%s'", strKey.c_str());
    break;
  }
  case OPERATOR: key.setOperatorKey(sb.ReadInt32()); break;
  case PROVIDER: key.setProviderKey(sb.ReadInt32()); break;;
  case SERVICE:  key.setServiceKey(sb.ReadInt32()); break;
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
    smsc_log_debug(logger_, "deserialize DEL");
    std::string varName;
    sb.ReadString(varName);
    DelCommand *cmd = new DelCommand();
    cmd->setVarName(varName);
    return cmd;
  }
  case PC_SET: {
    smsc_log_debug(logger_, "deserialize SET");
    Property prop;
    prop.Deserialize(sb);
    smsc_log_debug(logger_, "property name=%s", prop.getName());
    SetCommand *cmd = new SetCommand();
    cmd->setProperty(prop);
    return cmd;
  }
  case PC_GET: {
    smsc_log_debug(logger_, "deserialize GET");
    std::string varName;
    sb.ReadString(varName);
    GetCommand *cmd = new GetCommand();
    cmd->setVarName(varName);
    return cmd;
  }
  case PC_INC:
  case PC_INC_RESULT: {
    smsc_log_debug(logger_, "deserialize INC");
    Property prop;
    prop.Deserialize(sb);
    IncCommand *cmd = new IncCommand();
    cmd->setProperty(prop);
    return cmd;
  }
  case PC_INC_MOD: {
    smsc_log_debug(logger_, "deserialize INC_MOD");
    uint32_t mod = sb.ReadInt32();
    Property prop;
    prop.Deserialize(sb);
    IncModCommand *cmd = new IncModCommand();
    cmd->setModulus(mod);
    cmd->setProperty(prop);
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

bool PersProtocol::SerialBufferResponseVisitor::visitAuthResponse(AuthResponse &resp) /* throw(PvapException) */  {
  return false;
}


bool PersProtocol::SerialBufferResponseVisitor::visitProfileResponse(ProfileResponse& resp)
{
    return resp.getResponse() ? resp.getResponse()->visit(*this) : false;
}


bool PersProtocol::SerialBufferResponseVisitor::visitBatchResponse(BatchResponse &resp) /* throw(PvapException) */  {
  SerialBufferResponseVisitor visitor(buff_);
  std::vector<BatchResponseComponent*> content = resp.getBatchContent();
  std::vector<BatchResponseComponent*>::iterator i = content.begin();
  for (; i != content.end(); ++i) {
    (*i)->visit( visitor );
  }
  return true;
}

uint8_t PersProtocol::SerialBufferResponseVisitor::getResponseStatus(uint8_t status) const {
  switch (status) {
  case Response::OK                 : return perstypes::RESPONSE_OK;
  case Response::ERROR              : return perstypes::RESPONSE_ERROR;
  case Response::PROPERTY_NOT_FOUND : return perstypes::RESPONSE_PROPERTY_NOT_FOUND;
  case Response::BAD_REQUEST        : return perstypes::RESPONSE_BAD_REQUEST;
  case Response::TYPE_INCONSISTENCE : return perstypes::RESPONSE_TYPE_INCONSISTENCE;
  case Response::NOT_SUPPORTED      : return perstypes::RESPONSE_NOTSUPPORT;
  case Response::SERVER_SHUTDOWN    : return perstypes::RESPONSE_ERROR;
  case Response::REQUEST_TIMEOUT    : return perstypes::RESPONSE_ERROR;
  case Response::UNKNOWN            : return 0;
    default: return status > Response::ERROR ? perstypes::RESPONSE_ERROR : 0;
  }
}


bool PersProtocol::SerialBufferResponseVisitor::SerialBufferResponseVisitor::visitDelResponse(DelResponse &resp) /* throw(PvapException) */  {
  buff_.WriteInt8(getResponseStatus(resp.getStatus()));
  return true;
}

bool PersProtocol::SerialBufferResponseVisitor::visitErrResponse(ErrorResponse &resp) /* throw(PvapException) */  {
  buff_.WriteInt8(getResponseStatus(resp.getStatus()));
  return true;
}

bool PersProtocol::SerialBufferResponseVisitor::visitGetResponse(GetResponse &resp) /* throw(PvapException) */  {
  buff_.WriteInt8(getResponseStatus(resp.getStatus()));
  resp.getProperty().Serialize(buff_);
  return true;
}

bool PersProtocol::SerialBufferResponseVisitor::visitIncResponse(IncResponse &resp) /* throw(PvapException) */  {
  buff_.WriteInt8(getResponseStatus(resp.getStatus()));
  buff_.WriteInt32(resp.getResult());
  return true;
}

bool PersProtocol::SerialBufferResponseVisitor::visitPingResponse(PingResponse &resp) /* throw(PvapException) */  {
  buff_.WriteInt8(getResponseStatus(resp.getStatus()));
  return true;
}

bool PersProtocol::SerialBufferResponseVisitor::visitSetResponse(SetResponse &resp) /* throw(PvapException) */  {
  buff_.WriteInt8(getResponseStatus(resp.getStatus()));
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
