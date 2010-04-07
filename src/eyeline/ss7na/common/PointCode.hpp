#ifndef __EYELINE_SS7NA_COMMON_POINTCODE_HPP__
# define __EYELINE_SS7NA_COMMON_POINTCODE_HPP__

# include <sys/types.h>
# include "eyeline/ss7na/common/types.hpp"

namespace eyeline {
namespace ss7na {
namespace common {

class PointCode {
public:
  PointCode()
  : _pcValue(0)
  {}
  explicit PointCode(point_code_t pc_value)
  : _pcValue(pc_value)
  {}

  uint8_t getMask() const { return uint8_t(_pcValue >> 24); }
  point_code_t getValue() const { return _pcValue; }

protected:
  point_code_t _pcValue;
};

class ANSI_PC : public PointCode {
public:
  explicit ANSI_PC(point_code_t pc_value)
  : PointCode(pc_value) {}

  ANSI_PC(uint8_t mask, uint8_t network, uint8_t cluster, uint8_t member)
  : PointCode((uint32_t(mask) << 24) | (uint32_t(network) << 16) | (uint32_t(cluster) << 8) | uint32_t(member))
  {}

  uint8_t getNetwork() const {
    return uint8_t((_pcValue >> 16) & 0xff);
  }

  uint8_t getCluster() const {
    return uint8_t((_pcValue >> 8) & 0xff);
  }

  uint8_t getMember() const {
    return uint8_t(_pcValue & 0xff);
  }
};

class ITU_PC : public PointCode {
public:
  ITU_PC::ITU_PC()
  {}

  ITU_PC::ITU_PC(point_code_t pc_value)
  : PointCode(pc_value)
  {}

  ITU_PC(uint8_t mask, uint8_t zone, uint8_t region, uint8_t sp)
  : PointCode((uint32_t(mask) << 24) | (uint32_t(zone) << 11) | (uint32_t(region) << 3) | (uint32_t(sp) & 0x07))
  {}

  uint8_t getZone() const
  {
    return uint8_t((_pcValue >> 11) & 0xff);
  }

  uint8_t getRegion() const
  {
    return uint8_t(_pcValue >> 3);
  }

  uint8_t getSP() const
  {
    return uint8_t(_pcValue & 0x7);
  }
};

}}}

#endif
