#ifndef __EYELINE_UTILX_TYPES_HPP__
# define __EYELINE_UTILX_TYPES_HPP__

# include <sys/types.h>

namespace eyeline {
namespace utilx {

struct variable_data_t {
  variable_data_t(const uint8_t* aData, uint16_t aDataLen)
    : data(aData), dataLen(aDataLen) {}
  const uint8_t* data;
  uint16_t dataLen;
};

}}

#endif
