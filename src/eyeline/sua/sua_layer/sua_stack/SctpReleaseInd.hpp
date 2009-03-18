#ifndef __EYELINE_SUA_SUALAYER_SUASTACK_SCTPRELEASEIND_HPP__
# define __EYELINE_SUA_SUALAYER_SUASTACK_SCTPRELEASEIND_HPP__

# include <eyeline/sua/sua_layer/io_dispatcher/IndicationPrimitive.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_stack {

class SctpReleaseInd : public io_dispatcher::IndicationPrimitive {
public:
  virtual uint32_t getIndicationTypeValue() const;

  virtual std::string toString() const;
private:
  static const int SCTP_RELEASE_IND = 0xF301;
};

}}}}

#endif
