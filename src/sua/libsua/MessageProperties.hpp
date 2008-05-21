#ifndef __SUA_LIBSUA_MESSAGEPROPERTIES_HPP__
# define __SUA_LIBSUA_MESSAGEPROPERTIES_HPP__ 1

# include <sys/types.h>

namespace libsua {

struct MessageProperties
{
  MessageProperties();

  bool returnOnError;

  uint32_t sequenceControlValue;

  uint8_t importance;

  uint8_t hopCount;

  typedef enum {SET_SEQUENCE_CONTROL = 0x01, SET_IMPORTANCE = 0x02, SET_HOP_COUNT = 0x04} field_mask_t;

  uint32_t fieldsMask;
};

}

#endif
