#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGES_SEGMENTATION_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGES_SEGMENTATION_HPP__

# include <string.h>
# include <sys/types.h>
# include "eyeline/ss7na/m3ua_gw/sccp/messages/OptionalParameters.hpp"
# include "eyeline/ss7na/common/TP.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace messages {

class Segmentation {
public:
  enum class_e {
    Class_0 = 0, Class_1 = 1
  };
  Segmentation() {}
  Segmentation(bool is_first_segment, class_e class_value,
               uint8_t remaining_segments, uint32_t local_ref) {
    _segmentation[0] = (uint8_t(is_first_segment) << 7) | (class_value << 6) |
                       (remaining_segments & 0x0f);

    _segmentation[1] = (local_ref >> 16) & 0xff;
    _segmentation[2] = (local_ref >> 8) & 0xff;
    _segmentation[3] = local_ref & 0xff;
  }

  size_t deserialize(const common::TP& packet_buf, size_t offset) {
    return common::extractField(packet_buf, offset, _segmentation, SEGMENTATION_VALUE_SIZE);
  }
  const uint8_t* getValue() const { return _segmentation; }

  bool isFirstSegment() const {
    return _segmentation[0] & 0x80;
  }

  class_e getClass() const {
    return class_e((_segmentation[0] & 0x40) >> 6);
  }

  uint8_t getRemainingSegments() const {
    return _segmentation[0] & 0x0f;
  }

  uint32_t getLocalReference() const {
    return _segmentation[1] << 16 | _segmentation[2] << 8 | _segmentation[3];
  }

  std::string toString() const {
    char segmentationStr[128];
    sprintf(segmentationStr, "{class_%d, %s, remaining segments=%d, localReference=%02X%02X%02X}", getClass(),
            (isFirstSegment()? "first segment" : "not first segment"),
            getRemainingSegments(),_segmentation[1], _segmentation[2], _segmentation[3]);

    return segmentationStr;
  }

private:
  uint8_t _segmentation[SEGMENTATION_VALUE_SIZE];
};

}}}}}

#endif
