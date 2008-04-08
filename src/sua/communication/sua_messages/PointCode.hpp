#ifndef __SUA_COMMUNICATION_SUAMESSAGES_POINTCODE_HPP__
# define __SUA_COMMUNICATION_SUAMESSAGES_POINTCODE_HPP__ 1

# include <sys/types.h>

namespace sua_messages {

class PointCode {
public:
  explicit PointCode(const uint8_t pcValue[]);
  virtual uint8_t getMask();

  union aligned_mem_value {
    uint32_t mem_boundary_aligner;
    uint8_t value[4];
  };

  const aligned_mem_value& getValue() const;
private:
  aligned_mem_value _pcBuf;
};

class ANSI_PC : public PointCode {
public:
  explicit ANSI_PC(const uint8_t pcValue[]);
  ANSI_PC(uint8_t mask, uint8_t network, uint8_t cluster, uint8_t member);

  uint8_t getNetwork() const;
  uint8_t getCluster() const;
  uint8_t getMember() const;
private:
  struct converter {
    converter(uint8_t mask, uint8_t network, uint8_t cluster, uint8_t member) {
      buf[0] = mask; buf[1] = network; buf[2] = cluster; buf[3] = member;
    }
    uint8_t buf[4];
  };
};

class ITU_PC : public PointCode {
public:
  explicit ITU_PC(const uint8_t pcValue[]);
  ITU_PC(uint8_t mask, uint8_t zone, uint8_t region, uint8_t sp);

  uint8_t getZone() const;
  uint8_t getRegion() const;
  uint8_t getSP() const;
private:
  struct converter {
    converter(uint8_t mask, uint8_t zone, uint8_t region, uint8_t sp) {
      buf[0] = mask; buf[1] = 0;
      uint8_t majorRegPart = region >> 3, minorRegPart = region & 0x1F;
      buf[2] = (zone << 3) | majorRegPart;  buf[3] = (minorRegPart << 3) | sp;
    }
    uint8_t buf[4];
  };
};

}

#endif
