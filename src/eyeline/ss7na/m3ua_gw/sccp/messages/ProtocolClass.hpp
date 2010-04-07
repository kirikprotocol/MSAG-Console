#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGES_PROTOCOLCLASS_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGES_PROTOCOLCLASS_HPP__

# include <sys/types.h>
# include <string>

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace messages {

typedef enum { CLASS0_CONNECIONLESS=0, CLASS1_CONNECIONLESS=1,
               CLASS2_CONNECION_ORIENTED=2, CLASS3_CONNECION_ORIENTED=3 } protocol_class_t;
typedef enum { NO_SPECIAL_OPTION=0, RETURN_MSG_ON_ERROR=1 } ret_on_err_ind_t;

class ProtocolClass {
public:
  ProtocolClass(uint8_t value) {
    _protocolClass = protocol_class_t(value & 0x03);
    _retOnErr = (value & 0x80) ? RETURN_MSG_ON_ERROR : NO_SPECIAL_OPTION;
  }
  ProtocolClass(protocol_class_t protocol_class, ret_on_err_ind_t ret_on_err)
  : _protocolClass(protocol_class), _retOnErr(ret_on_err)
  {}

  std::string toString() const;

  protocol_class_t getProtocolClassValue() const { return _protocolClass; }
  ret_on_err_ind_t getRetOnErrorIndication() const { return _retOnErr; }
  uint8_t getValue() const {
    if ( _retOnErr )
      return _protocolClass | 0x80;
    else
      return _protocolClass;
  }

private:
  protocol_class_t _protocolClass;
  ret_on_err_ind_t _retOnErr;
};

}}}}}

#endif
