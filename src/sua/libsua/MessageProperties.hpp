#ifndef __SUA_LIBSUA_MESSAGEPROPERTIES_HPP__
# define __SUA_LIBSUA_MESSAGEPROPERTIES_HPP__ 1

# include <sys/types.h>
# include <sua/libsua/Segmentation.hpp>

namespace libsua {

struct MessageProperties
{
  MessageProperties();

  uint8_t protocolClass;

  bool returnOnError;

  uint32_t sequenceControlValue;

  uint8_t importance;

  uint8_t messagePriority;

  uint32_t correlationId;

  Segmentation segmentation;

  uint8_t hopCount;

  typedef enum {SET_PROT_CLASS = 0x01, SET_RETURN_ON_ERROR = 0x02, SET_SEQUENCE_CONTROL = 0x04,
                SET_SEGMENTATION = 0x08, SET_IMPORTANCE = 0x10, SET_MESSAGE_PRIORITY = 0x20,
                SET_CORRELATION_ID = 0x40, SET_HOP_COUNT = 0x80} field_mask_t;

  uint32_t fieldsMask;
};

}

#endif
