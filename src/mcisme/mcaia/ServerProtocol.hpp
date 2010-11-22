#ifndef __SMSC_MCISME_MCAIA_SERVERPROTOCOL_HPP__
#define __SMSC_MCISME_MCAIA_SERVERPROTOCOL_HPP__ 1
#include "eyeline/protogen/framework/SerializerBuffer.hpp"
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "mcisme/IAProtocolHandler.hpp"
#include "mcisme/mcaia/BusyRequest.hpp"

namespace smsc {
namespace mcisme {
namespace mcaia {

class ServerProtocol{
public:
  enum{
    tag_BusyRequest=1,
    tag_BusyResponse=2
  };
 
  ServerProtocol():handler(0)
  {
  }

  void assignHandler(IAProtocolHandler* newHandler);
  void decodeAndHandleMessage(const char* buf,size_t sz);
  void decodeAndHandleMessage(eyeline::protogen::framework::SerializerBuffer& ss);

  template <class MsgType>
  void encodeMessage(const MsgType& msg,eyeline::protogen::framework::SerializerBuffer* ss)
  {
    ss->writeInt32(4+4+msg.template length<eyeline::protogen::framework::SerializerBuffer>());
    ss->writeInt32(msg.messageGetTag());
    ss->writeInt32(msg.messageGetSeqNum());
    msg.serialize(*ss);
  }

protected:
  IAProtocolHandler* handler;
};

}
}
}

#endif
