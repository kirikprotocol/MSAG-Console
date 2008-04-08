#ifndef __SUA_SUALAYER_SUAUSERCOMMUNICATION_TCPESTABLISHIND_HPP__
# define __SUA_SUALAYER_SUAUSERCOMMUNICATION_TCPESTABLISHIND_HPP__ 1

# include <sua/sua_layer/io_dispatcher/IndicationPrimitive.hpp>

namespace sua_user_communication {

class TcpEstablishInd : public io_dispatcher::IndicationPrimitive {
public:
  virtual uint32_t getIndicationTypeValue() const;

  virtual std::string toString() const;
private:
  static const uint32_t TCP_ESTABLISH_IND=0xFF000001;
};

}

#endif
