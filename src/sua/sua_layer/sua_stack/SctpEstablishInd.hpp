#ifndef __SUA_SUALAYER_SUASTACK_SCTPESTABLISHIND_HPP__
# define __SUA_SUALAYER_SUASTACK_SCTPESTABLISHIND_HPP__ 1

# include <sua/sua_layer/io_dispatcher/IndicationPrimitive.hpp>

namespace sua_stack {

class SctpEstablishInd : public io_dispatcher::IndicationPrimitive {
public:
  virtual uint32_t getIndicationTypeValue() const;

  virtual std::string toString() const;
private:
  static const int SCTP_CONNECTION_IND = 0xF301;
};

}

#endif
