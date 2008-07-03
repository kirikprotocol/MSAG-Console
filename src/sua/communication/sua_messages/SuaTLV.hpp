#ifndef __SUA_COMMUNICATION_SUAMESSAGES_SUATLV_HPP__
# define __SUA_COMMUNICATION_SUAMESSAGES_SUATLV_HPP__ 1

# include <sua/communication/TP.hpp>
# include <sua/utilx/Exception.hpp>
# include <sys/types.h>
# include <sua/communication/sua_messages/PointCode.hpp>
# include <sua/communication/sua_messages/SSN.hpp>
# include <sua/communication/sua_messages/ProtocolClass.hpp>
# include <sua/communication/sua_messages/GlobalTitle.hpp>
# include <sua/communication/types.hpp>

namespace sua_messages {

class SuaTLV {
public:
  explicit SuaTLV(uint16_t tag) : _tag(tag) {}

  virtual ~SuaTLV() {}

  virtual size_t serialize(communication::TP* packetBuf,
                           size_t offset /*position inside buffer where TLV object will be stored*/) const;

  virtual size_t deserialize(const communication::TP& packetBuf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t valLen) = 0;

  virtual uint16_t getLength() const = 0;

  virtual uint16_t getActualLength() const { return getLength(); }

  uint16_t getTag() const { return _tag; }

  virtual bool isSetValue() const = 0;

  virtual std::string toString() const = 0;
protected:
  enum { HEADER_SZ = sizeof(uint16_t) + sizeof(uint16_t) };
private:
  uint16_t _tag;
};

class TLV_IntegerPrimitive : public SuaTLV {
public:
  explicit TLV_IntegerPrimitive(uint16_t tag) : SuaTLV(tag), _value(0), _isValueSet(false) {}
  TLV_IntegerPrimitive(uint16_t tag, uint32_t val) : SuaTLV(tag), _value(val), _isValueSet(true) {}

  virtual size_t serialize(communication::TP* packetBuf,
                           size_t offset /*position inside buffer where TLV object will be stored*/) const;

  virtual size_t deserialize(const communication::TP& packetBuf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t valLen);

  virtual uint16_t getLength() const;

  virtual uint32_t getValue() const;

  virtual bool isSetValue() const;
private:
  uint32_t _value; // value stored in host bytes order
  bool _isValueSet;
};


template <size_t SZ>
class TLV_StringPrimitive : public SuaTLV {
public:
  explicit TLV_StringPrimitive(uint16_t tag) : SuaTLV(tag),  _valLen(0), _isValueSet(false) {
    memset(_value, 0, sizeof(_value));
  }

  TLV_StringPrimitive(uint16_t tag, const char* val, uint16_t valLen) : SuaTLV(tag), _valLen(valLen), _isValueSet(true) {
    if ( _valLen > sizeof(_value) )
      throw smsc::util::Exception("TLV_StringPrimitive::TLV_StringPrimitive::: value length [=%d] exceeded max. allowable value [=%d]", _valLen, SZ);
    memcpy(_value, val, _valLen); _value[_valLen] = 0;
  }

  virtual size_t serialize(communication::TP* packetBuf,
                           size_t offset /*position inside buffer where TLV object will be stored*/) const;

  virtual size_t deserialize(const communication::TP& packetBuf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t valLen);

  virtual uint16_t getLength() const;

  std::string getPrintableValue() const;

  virtual bool isSetValue() const;
private:
  char _value[SZ];
  uint16_t _valLen;
  bool _isValueSet;

  int getUtf8Len(uint8_t firstUtf8Octet) const;
};

enum {MAX_OCTET_ARRAY_SIZE=65536};

template <size_t SZ>
class TLV_OctetArrayPrimitive : public SuaTLV {
public:
  explicit TLV_OctetArrayPrimitive(uint16_t tag);

  TLV_OctetArrayPrimitive(uint16_t tag, const uint8_t* val, uint16_t valLen);

  virtual size_t serialize(communication::TP* packetBuf,
                           size_t offset /*position inside buffer where TLV object will be stored*/) const;

  virtual size_t deserialize(const communication::TP& packetBuf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t valLen);

  virtual uint16_t getLength() const;

  virtual uint16_t getActualLength() const;

  const uint8_t* getValue() const;

  virtual bool isSetValue() const;

  uint16_t getValueLength() const;

protected:
  TLV_OctetArrayPrimitive(uint16_t tag, size_t reservedOctetsOffset, const uint8_t* val, uint16_t valLen);
private:
  union aligner {
    uint32_t _mem_boundary_aligner;
    uint8_t _value[SZ];
  } _valueBuffer;

  uint16_t _valLen;
  bool _isValueSet;
  uint16_t _paddingLen;
};

class TLV_ApplicationStatus : public TLV_IntegerPrimitive {
public:
  TLV_ApplicationStatus();
  TLV_ApplicationStatus(uint16_t statusType, uint16_t statusId);

  virtual std::string toString() const;

  uint16_t getStatusType() const;
  uint16_t getStatusId() const;

  enum { AS_STATE_CHANGE = 1, OTHER = 2 };
  enum { ASP_INACTIVE_NOTICE = 2, ASP_ACTIVE_NOTICE = 3, ASP_PENDING_NOTICE = 4 };
private:
  static const uint16_t TAG = 0x000D;
};

class TLV_AspIdentifier : public TLV_IntegerPrimitive {
public:
  TLV_AspIdentifier();
  explicit TLV_AspIdentifier(uint32_t aspId);
  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x0011;
};

class TLV_InfoString : public TLV_StringPrimitive<255> {
public:
  TLV_InfoString();
  explicit TLV_InfoString(const std::string& str);
  virtual std::string toString() const;

private:
  static const uint16_t TAG = 0x0004;
};

class TLV_DiagnosticInformation : public TLV_OctetArrayPrimitive<MAX_OCTET_ARRAY_SIZE> {
public:
  TLV_DiagnosticInformation();
  explicit TLV_DiagnosticInformation(const std::string& str);
  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x0007;
};

class TLV_ErrorCode : public TLV_IntegerPrimitive {
public:
  TLV_ErrorCode();
  explicit TLV_ErrorCode(uint32_t errCode);
  virtual size_t deserialize(const communication::TP& packetBuf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t valLen);
  virtual std::string toString() const;

  const char* getErrorCodeDescription() const;
private:
  static const uint16_t TAG = 0x000C;
};

class TLV_TrafficModeType : public TLV_IntegerPrimitive {
public:
  TLV_TrafficModeType();
  explicit TLV_TrafficModeType(uint32_t trafficMode);
  virtual size_t deserialize(const communication::TP& packetBuf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t valLen);
  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x000B;
};

class TLV_DRNLabel : public TLV_OctetArrayPrimitive<sizeof(uint32_t)> {
public:
  TLV_DRNLabel();
  TLV_DRNLabel(uint8_t startLabelPosition, uint8_t endLabelPosition, uint16_t labelValue);
  virtual size_t deserialize(const communication::TP& packetBuf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t valLen);
  virtual std::string toString() const;

  uint8_t getStartLabelPosition() const;
  uint8_t getEndLabelPosition() const;
  uint16_t getLabelValue() const;
private:
  struct temporary_buf {
    temporary_buf(uint8_t startLabelPosition, uint8_t endLabelPosition, uint16_t labelValue);
    uint8_t array[sizeof(uint32_t)];
  };

  static const uint16_t TAG = 0x010F;
};

class TLV_TIDLabel : public TLV_OctetArrayPrimitive<sizeof(uint32_t)> {
public:
  TLV_TIDLabel();
  TLV_TIDLabel(uint8_t startLabelPosition, uint8_t endLabelPosition, uint16_t labelValue);
  virtual size_t deserialize(const communication::TP& packetBuf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t valLen);
  virtual std::string toString() const;

  uint8_t getStartLabelPosition() const;
  uint8_t getEndLabelPosition() const;
  uint16_t getLabelValue() const;
private:
  struct temporary_buf {
    temporary_buf(uint8_t startLabelPosition, uint8_t endLabelPosition, uint16_t labelValue);
    uint8_t array[sizeof(uint32_t)];
  };

  static const uint16_t TAG = 0x0110;
};

class TLV_RoutingContext : public TLV_OctetArrayPrimitive<MAX_OCTET_ARRAY_SIZE> {
public:
  TLV_RoutingContext();
  TLV_RoutingContext(uint32_t indexes[] , size_t numOfIndexes);
  virtual size_t deserialize(const communication::TP& packetBuf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t valLen);
  virtual std::string toString() const;

  uint32_t getIndexValue(size_t idxNum) const;
  size_t getMaxIndexNum() const;
private:
  struct temporary_buf {
    temporary_buf(uint32_t indexes[] , size_t numOfIndexes);

    union ValueBuffer {
      uint32_t arrayUint32[MAX_OCTET_ARRAY_SIZE / sizeof(uint32_t)];
      uint8_t array[MAX_OCTET_ARRAY_SIZE];
    } valBuf;
  };

  size_t _numOfIndexes;
  static const uint16_t TAG = 0x0006;
};

class TLV_NetworkAppearance : public TLV_IntegerPrimitive {
public:
  TLV_NetworkAppearance();
  explicit TLV_NetworkAppearance(uint32_t networkAppearance);

  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x010D;
};

class TLV_AffectedPointCode : public TLV_OctetArrayPrimitive<MAX_OCTET_ARRAY_SIZE> {
public:
  TLV_AffectedPointCode();
  explicit TLV_AffectedPointCode(const PointCode& pc);

  virtual std::string toString() const;

  bool getNextPC(ANSI_PC* pointCode);
  bool getNextPC(ITU_PC* pointCode);
private:
  size_t _nextPCOffset;
  static const uint16_t TAG = 0x0012;
};

class TLV_SSN : public TLV_IntegerPrimitive {
public:
  TLV_SSN();
  explicit TLV_SSN(uint8_t ssnValue);

  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x8003;
};

class TLV_SMI : public TLV_IntegerPrimitive {
public:
  TLV_SMI();
  explicit TLV_SMI(uint8_t smi);
  virtual size_t deserialize(const communication::TP& packetBuf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t valLen);
  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x0112;
};

class TLV_UserCause : public TLV_IntegerPrimitive {
public:
  TLV_UserCause();
  TLV_UserCause(uint16_t cause, uint16_t user);
  virtual size_t deserialize(const communication::TP& packetBuf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t valLen);
  virtual std::string toString() const;

  uint16_t getUser() const;
  uint16_t getCause() const;
private:
  static const uint16_t TAG = 0x010C;
};

class TLV_CongestionLevel : public TLV_IntegerPrimitive {
public:
  TLV_CongestionLevel();
  explicit TLV_CongestionLevel(uint8_t congestionLevel);
  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x0118;
};

class TLV_ProtocolClass : public TLV_IntegerPrimitive {
public:
  TLV_ProtocolClass();
  TLV_ProtocolClass(const ProtocolClass& protocolClass);

  ProtocolClass getProtocolClassValue() const;
  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x0115;
};

class TLV_SequenceControl : public TLV_IntegerPrimitive {
public:
  TLV_SequenceControl();
  explicit TLV_SequenceControl(uint32_t sequenceControl);
  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x0116;
};

class TLV_Segmentation : public TLV_OctetArrayPrimitive<sizeof(uint32_t)> {
public:
  TLV_Segmentation();
  TLV_Segmentation(bool isFirstSegment, uint8_t numOfRemainingSegments, uint32_t reference);
  virtual std::string toString() const;

  bool isFirstSegment() const;
  uint8_t getNumOfRemainingSegments() const;
  uint32_t getReferenceValue() const;
private:
  struct temporary_buf {
    temporary_buf(bool isFirstSegment, uint8_t numOfRemainingSegments, uint32_t reference);
    uint8_t array[sizeof(uint32_t)];
  };
  static const uint16_t TAG = 0x0117;
};

class TLV_SS7HopCount : public TLV_IntegerPrimitive {
public:
  TLV_SS7HopCount();
  TLV_SS7HopCount(uint8_t hopCount);
  virtual std::string toString() const;

  uint8_t getHopCountValue() const;
private:
  static const uint16_t TAG = 0x0101;
};

class TLV_Importance : public TLV_IntegerPrimitive {
public:
  TLV_Importance();
  TLV_Importance(uint8_t importance);
  virtual size_t deserialize(const communication::TP& packetBuf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t valLen);
  virtual std::string toString() const;

  uint8_t getImportanceValue() const;
private:
  static const uint16_t TAG = 0x0113;
};

class TLV_MessagePriority : public TLV_IntegerPrimitive {
public:
  TLV_MessagePriority();
  TLV_MessagePriority(uint8_t priority);
  virtual size_t deserialize(const communication::TP& packetBuf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t valLen);
  virtual std::string toString() const;

  uint8_t getMessagePriorityValue() const;
private:
  static const uint16_t TAG = 0x0114;
};

class TLV_CorrelationId : public TLV_IntegerPrimitive {
public:
  TLV_CorrelationId();
  TLV_CorrelationId(uint32_t correlationId);
  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x0013;
};

class TLV_Data : public TLV_OctetArrayPrimitive<MAX_OCTET_ARRAY_SIZE> {
public:
  TLV_Data();
  explicit TLV_Data(const uint8_t* val, uint16_t len);
  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x010B;
};

class TLV_PointCode : public TLV_OctetArrayPrimitive<sizeof(uint32_t)> {
public:
  TLV_PointCode();
  explicit TLV_PointCode(const ANSI_PC& pc);
  explicit TLV_PointCode(const ITU_PC& pc);

  virtual std::string toString() const;

  ANSI_PC get_ANSI_PC() const;
  ITU_PC get_ITU_PC() const;
private:
  static const uint16_t TAG = 0x8002;
};

enum { GT_TOTAL_MAX_SZ = 32 };

class TLV_GlobalTitle : public TLV_OctetArrayPrimitive<GT_TOTAL_MAX_SZ> {
public:
  TLV_GlobalTitle();
  TLV_GlobalTitle(const GlobalTitle& gt);

  virtual size_t deserialize(const communication::TP& packetBuf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t valLen);
  virtual std::string toString() const;

  const GlobalTitle& getGlobalTitleValue() const;
private:
  GlobalTitle _globalTitle;

  static const uint16_t TAG = 0x8001;
  static const size_t RESERVED_OCTETS = 3;
};

class TLV_Address : public SuaTLV {
public:
  explicit TLV_Address(uint16_t tag);
  TLV_Address(uint16_t tag, const TLV_PointCode& pointCode, const TLV_SSN& ssn);
  TLV_Address(uint16_t tag, const TLV_GlobalTitle& gt, const TLV_SSN& ssn);
  TLV_Address(uint16_t tag, const TLV_GlobalTitle& gt);

  virtual size_t serialize(communication::TP* packetBuf,
                           size_t offset /*position inside buffer where tag's data will be stored*/) const;

  virtual size_t deserialize(const communication::TP& packetBuf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t valLen) ;

  virtual uint16_t getLength() const;

  virtual std::string toString() const;

  typedef enum { RESERVED = 0x00, ROUTE_ON_GT=0x01, ROUTE_ON_SSN_PLUS_PC=0x02,
                 ROUTE_ON_HOSTNAME=0x03, ROUTE_ON_SSN_PLUS_IP_ADDRESS=0x04 } routing_indicator_t;
  uint16_t getRoutingIndicator() const;

  typedef enum { SSN_INCLUDE_INDICATION = 0x01, PC_INCLUDE_INDICATION = 0x02, GT_INCLUDE_INDICATION = 0x04} address_indicator_t;
  uint16_t getAddressIndicator() const;
  void setAddressIndicator(uint16_t addrIndicator);

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
  TLV_SourceAddress(const TLV_PointCode& pointCode, const TLV_SSN& ssn);
  TLV_SourceAddress(const TLV_GlobalTitle& gt, const TLV_SSN& ssn);
  TLV_SourceAddress(const TLV_GlobalTitle& gt);

  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x0102;
};

class TLV_DestinationAddress : public TLV_Address {
public:
  TLV_DestinationAddress();
  TLV_DestinationAddress(const TLV_PointCode& pointCode, const TLV_SSN& ssn);
  TLV_DestinationAddress(const TLV_GlobalTitle& gt, const TLV_SSN& ssn);
  TLV_DestinationAddress(const TLV_GlobalTitle& gt);

  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x0103;
};

class TLV_SCCP_Cause : public TLV_OctetArrayPrimitive<sizeof(uint32_t)> {
public:
  TLV_SCCP_Cause();
  TLV_SCCP_Cause(communication::return_cause_type_t causeType, uint8_t causeValue);
  virtual size_t deserialize(const communication::TP& packetBuf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t valLen);
  virtual std::string toString() const;

  communication::return_cause_type_t getCauseType() const;

  uint8_t getCauseValue() const;
private:
  struct temporary_buf {
    temporary_buf(uint8_t causeType, uint8_t causeValue);
    uint8_t array[sizeof(uint32_t)];
  };

  static const uint16_t TAG = 0x0106;
};

class SuaTLVFactory {
public:
  SuaTLVFactory();

  void registerExpectedOptionalTlv(SuaTLV* expectedTlv);
  void registerExpectedMandatoryTlv(SuaTLV* expectedTlv);

  size_t parseInputBuffer(const communication::TP& packetBuf, size_t offset);
private:
  static const int NUM_OF_TAGS = 0x0019 + 0x0119 - 0x0100 + 0x8007 - 0x8000;

  SuaTLV* _optionalTlvTypeToTLVObject[NUM_OF_TAGS];
  SuaTLV* _mandatoryTlvTypeToTLVObject[NUM_OF_TAGS];
  int _numsOfMandatoryTlvObjects;

  unsigned int getTagIdx(uint16_t tag);
  void setPositionTo4BytesBoundary(size_t* offset);
  void generateProtocolException();
};

}

#endif
