#ifndef __SUA_COMMUNICATION_SUAMESSAGES_PROTOCOLCLASS_HPP__
# define __SUA_COMMUNICATION_SUAMESSAGES_PROTOCOLCLASS_HPP__ 1

# include <sys/types.h>
# include <string>

namespace sua_messages {

typedef enum { CLASS0_CONNECIONLESS=0, CLASS1_CONNECIONLESS=1,
                 CLASS2_CONNECION_ORIENTED=2, CLASS3_CONNECION_ORIENTED=3 } protocol_class_t;
typedef enum { NO_SPECIAL_OPTION=0, RETURN_MSG_ON_ERROR =1 } ret_on_err_ind_t;

class ProtocolClass {
public:
  ProtocolClass(uint32_t value);
  ProtocolClass(protocol_class_t protocolClass, ret_on_err_ind_t retOnErr);
  std::string toString() const;
  protocol_class_t getProtocolClassValue() const;
  ret_on_err_ind_t getRetOnErrorIndication() const;
  operator uint32_t() const;
private:
  protocol_class_t _protocolClass;
  ret_on_err_ind_t _retOnErr;
};

}

#endif
