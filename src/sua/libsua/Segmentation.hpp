#ifndef __SUA_LIBSUA_SEGMENTATION_HPP__
# define __SUA_LIBSUA_SEGMENTATION_HPP__ 1

# include <sys/types.h>

namespace libsua {

struct Segmentation
{
  Segmentation();

  Segmentation(uint32_t aSegmentationReference, bool aFirstSegment, uint8_t remainSegments);

  uint32_t segmentationReference;

  bool firstSegment;

  uint8_t remainSegments;
};

}

#endif
