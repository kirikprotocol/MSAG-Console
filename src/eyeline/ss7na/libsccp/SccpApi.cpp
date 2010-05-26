#include "SccpApi.hpp"

namespace eyeline {
namespace ss7na {
namespace libsccp {

SccpApi::CallResult
SccpApi::unitdata_req(const uint8_t* message,
                      uint16_t message_size,
                      const sccp::SCCPAddress& called_addr,
                      const sccp::SCCPAddress& calling_addr,
                      const MessageProperties& msg_properties)
{
  uint8_t calledAddr[256], callingAddr[256];
  uint8_t calledAddrLen, callingAddrLen;

  calledAddrLen = called_addr.pack2Octs(calledAddr);
  callingAddrLen = calling_addr.pack2Octs(callingAddr);

  return unitdata_req(message, message_size,
                      calledAddr, calledAddrLen,
                      callingAddr, callingAddrLen,
                      msg_properties);
}

SccpApi::ErrorCode_e
SccpApi::unitdata_req(const uint8_t* message,
                      uint16_t message_size,
                      const sccp::SCCPAddress& called_addr,
                      const sccp::SCCPAddress& calling_addr,
                      const MessageProperties& msg_properties,
                      unsigned int connect_num)
{
  uint8_t calledAddr[256], callingAddr[256];
  uint8_t calledAddrLen, callingAddrLen;

  calledAddrLen = called_addr.pack2Octs(calledAddr);
  callingAddrLen = calling_addr.pack2Octs(callingAddr);

  return unitdata_req(message, message_size,
                      calledAddr, calledAddrLen,
                      callingAddr, callingAddrLen,
                      msg_properties, connect_num);
}

}}}
