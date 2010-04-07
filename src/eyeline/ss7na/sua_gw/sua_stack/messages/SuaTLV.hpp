#ifndef __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_SUATLV_HPP__
# define __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_SUATLV_HPP__

# include <sys/types.h>
# include "eyeline/utilx/Exception.hpp"
# include "eyeline/ss7na/common/TP.hpp"
# include "eyeline/ss7na/common/types.hpp"
# include "eyeline/ss7na/common/AdaptationLayer_TLV.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/SSN.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/ProtocolClass.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/GlobalTitle.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

class TLV_DRNLabel : public common::TLV_OctetArrayPrimitive<sizeof(uint32_t)> {
public:
  TLV_DRNLabel();
  TLV_DRNLabel(uint8_t start_label_position, uint8_t end_label_position,
               uint16_t label_value);
  virtual size_t deserialize(const common::TP& packet_buf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t val_len);
  virtual std::string toString() const;

  uint8_t getStartLabelPosition() const;
  uint8_t getEndLabelPosition() const;
  uint16_t getLabelValue() const;
private:
  struct temporary_buf {
    temporary_buf(uint8_t start_label_position, uint8_t end_label_position,
                  uint16_t label_value);
    uint8_t array[sizeof(uint32_t)];
  };

  static const uint16_t TAG = 0x010F;
};

class TLV_TIDLabel : public common::TLV_OctetArrayPrimitive<sizeof(uint32_t)> {
public:
  TLV_TIDLabel();
  TLV_TIDLabel(uint8_t start_label_position, uint8_t end_label_position,
               uint16_t label_value);
  virtual size_t deserialize(const common::TP& packet_buf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t valLen);
  virtual std::string toString() const;

  uint8_t getStartLabelPosition() const;
  uint8_t getEndLabelPosition() const;
  uint16_t getLabelValue() const;
private:
  struct temporary_buf {
    temporary_buf(uint8_t start_label_position, uint8_t end_label_position,
                  uint16_t label_value);
    uint8_t array[sizeof(uint32_t)];
  };

  static const uint16_t TAG = 0x0110;
};

class TLV_NetworkAppearance : public common::TLV_IntegerPrimitive {
public:
  TLV_NetworkAppearance();
  explicit TLV_NetworkAppearance(uint32_t network_appearance);

  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x010D;
};

class TLV_SSN : public common::TLV_IntegerPrimitive {
public:
  TLV_SSN();
  explicit TLV_SSN(uint8_t ssnValue);

  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x8003;
};

class TLV_SMI : public common::TLV_IntegerPrimitive {
public:
  TLV_SMI();
  explicit TLV_SMI(uint8_t smi);
  virtual size_t deserialize(const common::TP& packet_buf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t val_len);
  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x0112;
};

class TLV_UserCause : public common::TLV_IntegerPrimitive {
public:
  TLV_UserCause();
  TLV_UserCause(uint16_t cause, uint16_t user);
  virtual size_t deserialize(const common::TP& packet_buf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t val_len);
  virtual std::string toString() const;

  uint16_t getUser() const;
  uint16_t getCause() const;
private:
  static const uint16_t TAG = 0x010C;
};

class TLV_CongestionLevel : public common::TLV_IntegerPrimitive {
public:
  TLV_CongestionLevel();
  explicit TLV_CongestionLevel(uint8_t congestion_level);
  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x0118;
};

class TLV_ProtocolClass : public common::TLV_IntegerPrimitive {
public:
  TLV_ProtocolClass();
  TLV_ProtocolClass(const ProtocolClass& protocol_class);

  ProtocolClass getProtocolClassValue() const;
  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x0115;
};

class TLV_SequenceControl : public common::TLV_IntegerPrimitive {
public:
  TLV_SequenceControl();
  explicit TLV_SequenceControl(uint32_t sequence_control);
  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x0116;
};

class TLV_Segmentation : public common::TLV_OctetArrayPrimitive<sizeof(uint32_t)> {
public:
  TLV_Segmentation();
  TLV_Segmentation(bool is_first_segment, uint8_t num_of_remaining_segments,
                   uint32_t reference);
  virtual std::string toString() const;

  bool isFirstSegment() const;
  uint8_t getNumOfRemainingSegments() const;
  uint32_t getReferenceValue() const;
private:
  struct temporary_buf {
    temporary_buf(bool is_first_segment, uint8_t num_of_remaining_segments,
                  uint32_t reference);
    uint8_t array[sizeof(uint32_t)];
  };
  static const uint16_t TAG = 0x0117;
};

class TLV_SS7HopCount : public common::TLV_IntegerPrimitive {
public:
  TLV_SS7HopCount();
  TLV_SS7HopCount(uint8_t hop_count);
  virtual std::string toString() const;

  uint8_t getHopCountValue() const;
private:
  static const uint16_t TAG = 0x0101;
};

class TLV_Importance : public common::TLV_IntegerPrimitive {
public:
  TLV_Importance();
  TLV_Importance(uint8_t importance);
  virtual size_t deserialize(const common::TP& packet_buf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t val_len);
  virtual std::string toString() const;

  uint8_t getImportanceValue() const;
private:
  static const uint16_t TAG = 0x0113;
};

class TLV_MessagePriority : public common::TLV_IntegerPrimitive {
public:
  TLV_MessagePriority();
  TLV_MessagePriority(uint8_t priority);
  virtual size_t deserialize(const common::TP& packet_buf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t val_len);
  virtual std::string toString() const;

  uint8_t getMessagePriorityValue() const;
private:
  static const uint16_t TAG = 0x0114;
};

class TLV_Data : public common::TLV_OctetArrayPrimitive<common::MAX_OCTET_ARRAY_SIZE> {
public:
  TLV_Data();
  explicit TLV_Data(const uint8_t* val, uint16_t len);
  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x010B;
};

class TLV_PointCode : public common::TLV_OctetArrayPrimitive<sizeof(uint32_t)> {
public:
  TLV_PointCode()
  : common::TLV_OctetArrayPrimitive<sizeof(uint32_t)>(TAG)
  {}

  explicit TLV_PointCode(const common::ANSI_PC& pc)
  : common::TLV_OctetArrayPrimitive<sizeof(uint32_t)>(TAG,
                                                      converter_from_ansi(pc.getMask(),
                                                                          pc.getNetwork(),
                                                                          pc.getCluster(),
                                                                          pc.getMember()).value,
                                                      static_cast<uint16_t>(sizeof(uint32_t)))
  {}

  explicit TLV_PointCode(const common::ITU_PC& pc)
  : common::TLV_OctetArrayPrimitive<sizeof(uint32_t)>(TAG,
                                                      converter_from_itu(pc.getMask(),
                                                                         pc.getZone(),
                                                                         pc.getRegion(),
                                                                         pc.getSP()).value,
                                                      static_cast<uint16_t>(sizeof(uint32_t)))
  {}

  virtual std::string toString() const;

  common::ANSI_PC get_ANSI_PC() const;
  common::ITU_PC get_ITU_PC() const;
private:
  struct converter_from_ansi {
    converter_from_ansi(uint8_t mask, uint8_t network, uint8_t cluster, uint8_t member) {
      value[0] = mask; value[1] = network; value[2] = cluster; value[3] = member;
    }
    uint8_t value[sizeof(uint32_t)];
  };
  struct converter_from_itu {
    converter_from_itu(uint8_t mask, uint8_t zone, uint8_t region, uint8_t sp) {
      value[0] = mask; value[1] = 0; value[2] = zone << 3 | (region >> 6); value[3] = (region << 2) | sp;
    }
    uint8_t value[sizeof(uint32_t)];
  };

  static const uint16_t TAG = 0x8002;
};

enum { GT_TOTAL_MAX_SZ = 32 };

class TLV_GlobalTitle : public common::TLV_OctetArrayPrimitive<GT_TOTAL_MAX_SZ> {
public:
  TLV_GlobalTitle();
  TLV_GlobalTitle(const GlobalTitle& gt);

  virtual size_t deserialize(const common::TP& packet_buf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t val_len);
  virtual std::string toString() const;

  const GlobalTitle& getGlobalTitleValue() const;
private:
  GlobalTitle _globalTitle;

  static const uint16_t TAG = 0x8001;
  static const size_t RESERVED_OCTETS = 3;
};

class TLV_Address : public common::AdaptationLayer_TLV {
public:
  explicit TLV_Address(uint16_t tag);
  TLV_Address(uint16_t tag, const TLV_PointCode& pointCode, const TLV_SSN& ssn);
  TLV_Address(uint16_t tag, const TLV_GlobalTitle& gt, const TLV_SSN& ssn);
  TLV_Address(uint16_t tag, const TLV_GlobalTitle& gt);

  virtual size_t serialize(common::TP* packet_buf,
                           size_t offset /*position inside buffer where tag's data will be stored*/) const;

  virtual size_t deserialize(const common::TP& packet_buf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t val_len) ;

  virtual uint16_t getLength() const;

  virtual std::string toString() const;

  typedef enum { RESERVED = 0x00, ROUTE_ON_GT=0x01, ROUTE_ON_SSN_PLUS_PC=0x02,
                 ROUTE_ON_HOSTNAME=0x03, ROUTE_ON_SSN_PLUS_IP_ADDRESS=0x04 } routing_indicator_t;
  uint16_t getRoutingIndicator() const;

  typedef enum { SSN_INCLUDE_INDICATION = 0x01, PC_INCLUDE_INDICATION = 0x02, GT_INCLUDE_INDICATION = 0x04} address_indicator_t;
  uint16_t getAddressIndicator() const;
  void setAddressIndicator(uint16_t addr_indicator);

  const TLV_PointCode& getPointCode() const;

  const TLV_SSN& getSSN() const;

  const TLV_GlobalTitle& getGlobalTitle() const;

  virtual bool isSetValue() const;

private:
  enum { INDICATORS_FIELDS_SZ = 4 };

  uint16_t _routingIndicator, _addressIndicator;
  TLV_PointCode _pointCode;
  TLV_SSN _ssn;
  TLV_GlobalTitle _gt;

  bool _isValueSet;
};

class TLV_SourceAddress : public TLV_Address {
public:
  TLV_SourceAddress();
  TLV_SourceAddress(const TLV_PointCode& point_code, const TLV_SSN& ssn);
  TLV_SourceAddress(const TLV_GlobalTitle& gt, const TLV_SSN& ssn);
  TLV_SourceAddress(const TLV_GlobalTitle& gt);

  virtual std::string toString() const;

private:
  static const uint16_t TAG = 0x0102;
};

class TLV_DestinationAddress : public TLV_Address {
public:
  TLV_DestinationAddress();
  TLV_DestinationAddress(const TLV_PointCode& point_code, const TLV_SSN& ssn);
  TLV_DestinationAddress(const TLV_GlobalTitle& gt, const TLV_SSN& ssn);
  TLV_DestinationAddress(const TLV_GlobalTitle& gt);

  virtual std::string toString() const;

private:
  static const uint16_t TAG = 0x0103;
};

class TLV_SCCP_Cause : public common::TLV_OctetArrayPrimitive<sizeof(uint32_t)> {
public:
  TLV_SCCP_Cause();
  TLV_SCCP_Cause(common::return_cause_type_t cause_type, uint8_t cause_value);
  virtual size_t deserialize(const common::TP& packet_buf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t val_len);
  virtual std::string toString() const;

  common::return_cause_type_t getCauseType() const;

  uint8_t getCauseValue() const;

private:
  struct temporary_buf {
    temporary_buf(uint8_t cause_type, uint8_t cause_value);
    uint8_t array[sizeof(uint32_t)];
  };

  static const uint16_t TAG = 0x0106;
};

static const int NUM_OF_TAGS = 0x0019 + 0x0119 - 0x0100 + 0x8007 - 0x8000;

class SuaTLVFactory : public common::AdaptationLayer_TLV_Factory<NUM_OF_TAGS> {
protected:
  virtual unsigned int getTagIdx(uint16_t tag);
};

}}}}}

#endif
