#ifndef __EYELINE_SS7NA_COMMON_ADAPTATIONLAYERTLV_HPP__
# define __EYELINE_SS7NA_COMMON_ADAPTATIONLAYERTLV_HPP__

# include <string.h>
# include <sys/types.h>
# include <string>

# include "eyeline/utilx/Exception.hpp"
# include "eyeline/ss7na/common/TP.hpp"

# include "eyeline/ss7na/common/PointCode.hpp"
# include "eyeline/ss7na/common/io_dispatcher/Exceptions.hpp"

namespace eyeline {
namespace ss7na {
namespace common {

class AdaptationLayer_TLV {
public:
  explicit AdaptationLayer_TLV(uint16_t tag) : _tag(tag) {}

  virtual ~AdaptationLayer_TLV() {}


  virtual size_t serialize(TP* packet_buf,
                           size_t offset /*position inside buffer where TLV object will be stored*/) const;

  virtual size_t deserialize(const TP& packet_buf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t val_len) = 0;

  virtual uint16_t getLength() const = 0;

  virtual uint16_t getActualLength() const { return getLength(); }

  uint16_t getTag() const { return _tag; }

  virtual bool isSetValue() const = 0;

  virtual std::string toString() const = 0;

protected:
  enum { HEADER_SZ = sizeof(uint16_t) + sizeof(uint16_t) };
  uint16_t _tag;
};

class TLV_IntegerPrimitive : public AdaptationLayer_TLV {
public:
  explicit TLV_IntegerPrimitive(uint16_t tag)
  : AdaptationLayer_TLV(tag), _value(0), _isValueSet(false) {}

  TLV_IntegerPrimitive(uint16_t tag, uint32_t val)
  : AdaptationLayer_TLV(tag), _value(val), _isValueSet(true) {}

  virtual size_t serialize(TP* packet_buf,
                           size_t offset /*position inside buffer where TLV object will be stored*/) const;

  virtual size_t deserialize(const TP& packet_buf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t val_len);

  virtual uint16_t getLength() const;

  virtual uint32_t getValue() const;

  virtual bool isSetValue() const;

private:
  uint32_t _value; // value stored in host bytes order
  bool _isValueSet;
};

template <size_t SZ>
class TLV_StringPrimitive : public AdaptationLayer_TLV {
public:
  explicit TLV_StringPrimitive(uint16_t tag)
  : AdaptationLayer_TLV(tag),  _valLen(0), _isValueSet(false) {
    memset(_value, 0, sizeof(_value));
  }

  TLV_StringPrimitive(uint16_t tag, const char* val, uint16_t val_len)
  : AdaptationLayer_TLV(tag), _valLen(val_len), _isValueSet(true) {
    if ( _valLen > sizeof(_value) )
      throw smsc::util::Exception("TLV_StringPrimitive::TLV_StringPrimitive::: value length [=%d] exceeded max. allowable value [=%d]", _valLen, SZ);
    memcpy(_value, val, _valLen); _value[_valLen] = 0;
  }

  virtual size_t serialize(TP* packet_buf,
                           size_t offset /*position inside buffer where TLV object will be stored*/) const;

  virtual size_t deserialize(const TP& packet_buf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t val_len);

  virtual uint16_t getLength() const;

  std::string getPrintableValue() const;

  virtual bool isSetValue() const;

private:
  char _value[SZ];
  uint16_t _valLen;
  bool _isValueSet;

  int getUtf8Len(uint8_t first_utf8_octet) const;
};

enum {MAX_OCTET_ARRAY_SIZE=65536};

template <size_t SZ>
class TLV_OctetArrayPrimitive : public AdaptationLayer_TLV {
public:
  explicit TLV_OctetArrayPrimitive(uint16_t tag);

  TLV_OctetArrayPrimitive(const TLV_OctetArrayPrimitive& rhs);

  TLV_OctetArrayPrimitive(uint16_t tag, const uint8_t* val, uint16_t val_len);

  TLV_OctetArrayPrimitive& operator=(const TLV_OctetArrayPrimitive& rhs);

  virtual size_t serialize(TP* packet_buf,
                           size_t offset /*position inside buffer where TLV object will be stored*/) const;

  virtual size_t deserialize(const TP& packet_buf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t val_len);

  virtual uint16_t getLength() const;

  virtual uint16_t getActualLength() const;

  const uint8_t* getValue() const;

  virtual bool isSetValue() const;

  uint16_t getValueLength() const;

protected:
  TLV_OctetArrayPrimitive(uint16_t tag, size_t reserved_octets_offset,
                          const uint8_t* val, uint16_t val_len);
private:
  union aligner {
    uint32_t _mem_boundary_aligner;
    uint8_t _value[SZ];
  } _valueBuffer;

  uint16_t _valLen;
  bool _isValueSet;
  uint16_t _paddingLen;
};

class TLV_InfoString : public TLV_StringPrimitive<255> {
public:
  TLV_InfoString();
  explicit TLV_InfoString(const std::string& str);
  virtual std::string toString() const;

private:
  static const uint16_t TAG = 0x0004;
};

class TLV_RoutingContext : public TLV_OctetArrayPrimitive<MAX_OCTET_ARRAY_SIZE> {
public:
  TLV_RoutingContext();
  TLV_RoutingContext(uint32_t indexes[] , size_t num_of_indexes);
  virtual size_t deserialize(const TP& packet_buf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t val_len);
  virtual std::string toString() const;

  uint32_t getIndexValue(size_t idx_num) const;
  size_t getMaxIndexNum() const;
private:
  struct temporary_buf {
    temporary_buf(uint32_t indexes[] , size_t num_of_indexes);

    union ValueBuffer {
      uint32_t arrayUint32[MAX_OCTET_ARRAY_SIZE / sizeof(uint32_t)];
      uint8_t array[MAX_OCTET_ARRAY_SIZE];
    } valBuf;
  };

  size_t _numOfIndexes;
  static const uint16_t TAG = 0x0006;
};

class TLV_DiagnosticInformation : public TLV_OctetArrayPrimitive<MAX_OCTET_ARRAY_SIZE> {
public:
  TLV_DiagnosticInformation();
  explicit TLV_DiagnosticInformation(const std::string& str);
  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x0007;
};

class TLV_TrafficModeType : public TLV_IntegerPrimitive {
public:
  TLV_TrafficModeType();
  explicit TLV_TrafficModeType(uint32_t traffic_mode);
  virtual size_t deserialize(const TP& packet_buf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t val_len);
  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x000B;
};

class TLV_ErrorCode : public TLV_IntegerPrimitive {
public:
  enum error_codes_e {
    InvalidVersion = 0x01, UnsupportedMessageClass = 0x03, UnsupportedMessageType = 0x04,
    UnsupportedTrafficModeType = 0x05, UnexpectedMessage = 0x06, ProtocolError = 0x07,
    InvalidStreamIdentifier = 0x09, RefusedManagementBlocking = 0x0d,
    ASPIdentifierRequired = 0x0e, InvalidASPIdentifier = 0x0f, InvalidParameterValue = 0x11,
    ParameterFieldError = 0x12, UnexpectedParameter = 0x13, DestinationStatusUnknown = 0x14,
    InvalidNetworkAppearance = 0x15, MissingParameter = 0x16, InvalidRoutingContext = 0x19,
    NoConfiguredASforASP = 0x1a, SubsystemStatusUnknown = 0x1b, InvalidLoadsharingLabel = 0x1c
  };

  TLV_ErrorCode();
  explicit TLV_ErrorCode(error_codes_e err_code);

  virtual size_t deserialize(const TP& packet_buf,
                             size_t offset /*position inside buffer where tag's data started*/,
                             uint16_t val_len);
  virtual std::string toString() const;

  const char* getErrorCodeDescription() const;
private:
  static const uint16_t TAG = 0x000C;
  static const char* _errCodeText[];
};

class TLV_ApplicationStatus : public TLV_IntegerPrimitive {
public:
  TLV_ApplicationStatus();
  TLV_ApplicationStatus(uint16_t status_type, uint16_t status_id);

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
  explicit TLV_AspIdentifier(uint32_t asp_id);
  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x0011;
};

class TLV_AffectedPointCode : public TLV_OctetArrayPrimitive<MAX_OCTET_ARRAY_SIZE> {
public:
  TLV_AffectedPointCode()
  : TLV_OctetArrayPrimitive<MAX_OCTET_ARRAY_SIZE>(TAG)
  {}

  explicit TLV_AffectedPointCode(const PointCode& pc)
  : TLV_OctetArrayPrimitive<MAX_OCTET_ARRAY_SIZE>(TAG, temporary_buf(pc.getValue()).array, static_cast<uint16_t>(sizeof(uint32_t)))
  {}

  virtual std::string toString() const;

  bool getNextPC(ANSI_PC* point_code, size_t* next_pc_offset) const;
  bool getNextPC(ITU_PC* point_code, size_t* next_pc_offset) const;

private:
  struct temporary_buf {
    temporary_buf(uint32_t pc_value) {
      pc_value &= 0x00ffffff;
      array[0] = 0;
      array[1] = uint8_t(pc_value >> 16);
      array[2] = uint8_t(pc_value >> 8);
      array[3] = uint8_t(pc_value);
    }
    uint8_t array[sizeof(uint32_t)];
  };

  static const uint16_t TAG = 0x0012;
};

class TLV_CorrelationId : public TLV_IntegerPrimitive {
public:
  TLV_CorrelationId();
  TLV_CorrelationId(uint32_t correlation_id);
  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x0013;
};

template <size_t NUM_OF_TAGS>
class AdaptationLayer_TLV_Factory {
public:
  AdaptationLayer_TLV_Factory();

  void registerExpectedOptionalTlv(AdaptationLayer_TLV* expected_tlv);
  void registerExpectedMandatoryTlv(AdaptationLayer_TLV* expected_tlv);

  size_t parseInputBuffer(const TP& packet_buf, size_t offset);

protected:
  virtual unsigned int getTagIdx(uint16_t tag) = 0;
  void setPositionTo4BytesBoundary(size_t* offset);
  void generateProtocolException();

private:
  AdaptationLayer_TLV* _optionalTlvTypeToTLVObject[NUM_OF_TAGS];
  AdaptationLayer_TLV* _mandatoryTlvTypeToTLVObject[NUM_OF_TAGS];
  int _numsOfMandatoryTlvObjects;
};

# include "eyeline/ss7na/common/AdaptationLayer_TLV_impl.hpp"

}}}

#endif
