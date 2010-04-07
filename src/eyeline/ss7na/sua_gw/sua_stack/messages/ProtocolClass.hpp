#ifndef __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_PROTOCOLCLASS_HPP__
# define __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_PROTOCOLCLASS_HPP__

# include <sys/types.h>
# include <string>

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

typedef enum { CLASS0_CONNECIONLESS=0, CLASS1_CONNECIONLESS=1,
                 CLASS2_CONNECION_ORIENTED=2, CLASS3_CONNECION_ORIENTED=3 } protocol_class_t;
typedef enum { NO_SPECIAL_OPTION=0, RETURN_MSG_ON_ERROR =1 } ret_on_err_ind_t;

class ProtocolClass {
public:
  ProtocolClass(uint32_t value);
  ProtocolClass(protocol_class_t protocol_class, ret_on_err_ind_t ret_on_rr);
  std::string toString() const;
  protocol_class_t getProtocolClassValue() const;
  ret_on_err_ind_t getRetOnErrorIndication() const;
  operator uint32_t() const;
private:
  protocol_class_t _protocolClass;
  ret_on_err_ind_t _retOnErr;
};

}}}}}

#endif
