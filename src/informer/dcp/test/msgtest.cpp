#include "../messages/AddDeliveryMessages.hpp"
#include "eyeline/protogen/framework/SerializerBuffer.hpp"
#include <stdio.h>

int main()
{
  try{
    using namespace eyeline::informer::dcp;
    messages::AddDeliveryMessages msg;
    msg.setDeliveryId(1);
    std::vector<messages::DeliveryMessage>& msgs=msg.getMessagesRef();
    messages::DeliveryMessage msg1;
    int i=0;
    for(int j=0;j<1000;j++)
    {
      char abnt[32];
      sprintf(abnt,"+79130%02d%04d",i,j);
      msg1.setAbonent(abnt);
      msg1.setMsgType(messages::MessageType::TextMessage);
      char txt[32];
      sprintf(txt,"this is message text:%dx%d",i,j);
      msg1.setText(txt);
      char ud[32];
      sprintf(ud,"ud%d:%d",i,j);
      msg1.setUserData(ud);
      msgs.push_back(msg1);
    }
    eyeline::protogen::framework::SerializerBuffer sb(200000);
    msg.serialize(sb);
    printf("%s\n",sb.getDump().c_str());
    messages::AddDeliveryMessages msg2;
    sb.rewind();
    msg2.deserialize(sb);
    printf("%s\n",msg2.toString().c_str());
  }catch(std::exception& e)
  {
    printf("exception:%s\n",e.what());
  }
  return 0;
}
