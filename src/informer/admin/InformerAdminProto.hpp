#ifndef __EYELINE_INFORMER_ADMIN_INFORMERADMINPROTO_HPP__
#define __EYELINE_INFORMER_ADMIN_INFORMERADMINPROTO_HPP__ 1
#include "eyeline/protogen/framework/SerializerBuffer.hpp"
#include "eyeline/protogen/framework/Exceptions.hpp"
#include "AdminCommandsHandler.hpp"
#include "messages/ConfigOpResult.hpp"
#include "messages/SetDefaultSmscResp.hpp"
#include "messages/LoggerGetCategoriesResp.hpp"
#include "messages/LoggerSetCategoriesResp.hpp"

namespace eyeline {
namespace informer {
namespace admin {

class InformerAdminProto{
public:
  enum{
    tag_ConfigOp=1,
    tag_ConfigOpResult=2,
    tag_SetDefaultSmsc=7,
    tag_SetDefaultSmscResp=8,
    tag_LoggerGetCategories=3,
    tag_LoggerGetCategoriesResp=4,
    tag_LoggerSetCategories=5,
    tag_LoggerSetCategoriesResp=6
  };
 
  InformerAdminProto():handler(0)
  {
  }

  void assignHandler(AdminCommandsHandler* newHandler);
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
  AdminCommandsHandler* handler;
};

}
}
}

#endif
