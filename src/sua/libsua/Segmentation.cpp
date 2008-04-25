#include "Segmentation.hpp"

namespace libsua {

Segmentation::Segmentation()
  : segmentationReference(0), firstSegment(0), remainSegments(0)
{}


Segmentation::Segmentation(uint32_t aSegmentationReference, bool aFirstSegment, uint8_t aRemainSegments)
  : segmentationReference(aSegmentationReference), firstSegment(aFirstSegment),
    remainSegments(aRemainSegments)
{}

}
