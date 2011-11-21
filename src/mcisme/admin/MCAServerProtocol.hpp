#ifndef __SMSC_MCISME_ADMIN_MCASERVERPROTOCOL_HPP__
#define __SMSC_MCISME_ADMIN_MCASERVERPROTOCOL_HPP__ 1
#include "eyeline/protogen/framework/SerializerBuffer.hpp"
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "MCAServerProtocolHandler.hpp"
#include "messages/FlushStatsResp.hpp"
#include "messages/GetStatsResp.hpp"
#include "messages/GetRunStatsResp.hpp"
#include "messages/GetSchedResp.hpp"
#include "messages/GetSchedListResp.hpp"
#include "messages/GetProfileResp.hpp"
#include "messages/SetProfileResp.hpp"

namespace smsc {
namespace mcisme {
namespace admin {

class MCAServerProtocol{
public:
  enum{
    tag_FlushStats=2,
    tag_FlushStatsResp=3,
    tag_GetStats=4,
    tag_GetStatsResp=5,
    tag_GetRunStats=6,
    tag_GetRunStatsResp=7,
    tag_GetSched=8,
    tag_GetSchedResp=9,
    tag_GetSchedList=12,
    tag_GetSchedListResp=13,
    tag_GetProfile=14,
    tag_GetProfileResp=15,
    tag_SetProfile=16,
    tag_SetProfileResp=17
  };
 
  MCAServerProtocol():handler(0)
  {
  }

  void assignHandler(MCAServerProtocolHandler* newHandler);
  void decodeAndHandleMessage(const char* buf,size_t sz,int connId);
  void decodeAndHandleMessage(eyeline::protogen::framework::SerializerBuffer& ss,int connId);

  template <class MsgType>
  void encodeMessage(const MsgType& msg,eyeline::protogen::framework::SerializerBuffer* ss)
  {
    ss->writeInt32(4+4+msg.template length<eyeline::protogen::framework::SerializerBuffer>());
    ss->writeInt32(msg.messageGetTag());
    ss->writeInt32(msg.messageGetSeqNum());
    msg.serialize(*ss);
  }

protected:
  MCAServerProtocolHandler* handler;
};

}
}
}

#endif
