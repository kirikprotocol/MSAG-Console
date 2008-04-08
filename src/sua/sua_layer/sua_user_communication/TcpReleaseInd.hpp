#ifndef __SUA_SUALAYER_SUAUSERCOMMUNICATION_TCPRELEASEIND_HPP__
# define __SUA_SUALAYER_SUAUSERCOMMUNICATION_TCPRELEASEIND_HPP__ 1

# include <sua/sua_layer/io_dispatcher/IndicationPrimitive.hpp>

namespace sua_user_communication {

class TcpReleaseInd : public io_dispatcher::IndicationPrimitive {
public:
  virtual uint32_t getIndicationTypeValue() const;

  virtual std::string toString() const;
private:
  static const int TCP_RELEASE_IND = 0xFF000002;
};

}

#endif
