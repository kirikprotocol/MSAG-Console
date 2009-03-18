#ifndef __EYELINE_SUA_SUALAYER_SUAUSERCOMMUNICATION_TCPESTABLISHIND_HPP__
# define __EYELINE_SUA_SUALAYER_SUAUSERCOMMUNICATION_TCPESTABLISHIND_HPP__

# include <eyeline/sua/sua_layer/io_dispatcher/IndicationPrimitive.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_user_communication {

class TcpEstablishInd : public io_dispatcher::IndicationPrimitive {
public:
  virtual uint32_t getIndicationTypeValue() const;

  virtual std::string toString() const;
private:
  static const uint32_t TCP_ESTABLISH_IND=0xFF000001;
};

}}}}

#endif
