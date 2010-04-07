#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_MESSAGES_M3UATLV_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_M3UASTACK_MESSAGES_M3UATLV_HPP__

# include <string.h>
# include <sys/types.h>

# include "eyeline/utilx/types.hpp"
# include "eyeline/utilx/Exception.hpp"
# include "eyeline/ss7na/common/TP.hpp"
# include "eyeline/ss7na/common/AdaptationLayer_TLV.hpp"
# include "eyeline/ss7na/common/PointCode.hpp"
# include "eyeline/ss7na/m3ua_gw/types.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace m3ua_stack {
namespace messages {

class TLV_NetworkAppearance : public common::TLV_IntegerPrimitive {
public:
  TLV_NetworkAppearance()
  : common::TLV_IntegerPrimitive(TAG)
  {}

  explicit TLV_NetworkAppearance(uint32_t network_appearance)
  : common::TLV_IntegerPrimitive(TAG, network_appearance)
  {}

  virtual std::string toString() const;
private:
  static const uint16_t TAG = 0x0200;
};

class TLV_UserCause : public common::TLV_IntegerPrimitive {
public:
  TLV_UserCause();
  TLV_UserCause(mtp_status_cause_e cause, mtp_user_identity_e user);
  virtual size_t deserialize(const common::TP& packet_buf,
                             size_t offset,
                             uint16_t val_len);
  virtual std::string toString() const;

  mtp_user_identity_e getUser() const;
  mtp_status_cause_e getCause() const;
private:
  static const uint16_t TAG = 0x0204;
};

class TLV_CongestionIndication : public common::TLV_IntegerPrimitive {
public:
  TLV_CongestionIndication()
  : common::TLV_IntegerPrimitive(TAG)
  {}

  TLV_CongestionIndication(mtp_congestion_level_e congestion_level)
  : common::TLV_IntegerPrimitive(TAG, congestion_level)
  {}

  virtual size_t deserialize(const common::TP& packet_buf,
                             size_t offset,
                             uint16_t val_len);

  virtual std::string toString() const;

private:
  static const uint16_t TAG = 0x0205;
};

class TLV_ConcernedDestination : public common::TLV_IntegerPrimitive {
public:
  TLV_ConcernedDestination()
  : common::TLV_IntegerPrimitive(TAG)
  {}

  TLV_ConcernedDestination(const common::PointCode& pc)
  : common::TLV_IntegerPrimitive(TAG, pc.getValue())
  {}

  virtual std::string toString() const;

  common::ANSI_PC getANSI_PC() const { return common::ANSI_PC(getValue()); }
  common::ITU_PC getITU_PC() const { return common::ITU_PC(getValue()); }

private:
  static const uint16_t TAG = 0x0206;
};

class TLV_ProtocolData : public common::AdaptationLayer_TLV {
public:
  TLV_ProtocolData();

  virtual size_t serialize(common::TP* packet_buf,
                           size_t offset) const;

  virtual size_t deserialize(const common::TP& packet_buf,
                             size_t offset,
                             uint16_t val_len);

  virtual uint16_t getLength() const;

  virtual bool isSetValue() const;

  virtual std::string toString() const;

  void setOPC(common::point_code_t opc) { _opc = opc; _isSetOpc = true; }
  common::point_code_t getOPC() const;

  void setDPC(common::point_code_t dpc) { _dpc = dpc; _isSetDpc = true; }
  common::point_code_t getDPC() const;

  void setServiceIndicator(uint8_t si) { _si = si; _isSetSi = true; }
  uint8_t getServiceIndicator() const;

  void setNetworkIndicator(uint8_t ni) { _ni = ni; _isSetNi = true; }
  uint8_t getNetworkIndicator() const;

  void setMessagePriority(uint8_t mp) { _messagePriority = mp; _isSetMessagePriority = true; }
  uint8_t getMessagePriority() const;

  void setSLS(uint8_t sls) { _sls = sls; _isSetSls = true; }
  uint8_t getSLS() const;

  void setUserProtocolData(const uint8_t* val, uint16_t len);
  utilx::variable_data_t getUserProtocolData() const;

private:
  common::point_code_t _opc, _dpc;
  bool _isSetOpc, _isSetDpc;
  uint8_t _si, _ni, _messagePriority, _sls;
  bool _isSetSi, _isSetNi, _isSetMessagePriority, _isSetSls;
  bool _isSetUserProtocolData;

  enum {EXPECTED_MAX_USER_PROTO_DATA_SIZE = 2048, SIZE_OF_FIXED_FIELDS=12};

  template<uint16_t SZ>
  struct MemoryAllocator {
    MemoryAllocator(uint16_t sz=0);
    MemoryAllocator(MemoryAllocator& rhs);
    MemoryAllocator& operator=(MemoryAllocator& rhs);
    ~MemoryAllocator();
    void copyObject(MemoryAllocator& rhs);

    void append(const uint8_t* value, size_t sz);
    void setValue(const uint8_t* value, size_t sz);
    const uint8_t* get() const;
    uint16_t getCurrentSize() const;
    void rewind();
    void setSize(uint16_t sz);

    union {
      uint8_t* bufferOnHeap;
      uint8_t bufferOnStack[SZ];
    } allocation;
    bool isBufferOnStack, externalBuff;
    uint16_t maxBufferSize, posInBuf;
  };

  MemoryAllocator<EXPECTED_MAX_USER_PROTO_DATA_SIZE> _userProtocolData;

  static const uint16_t TAG = 0x0210;
};

static const int NUM_OF_TAGS = 0x0014 + 0x0214 - 0x0200;

class M3uaTLVFactory : public common::AdaptationLayer_TLV_Factory<NUM_OF_TAGS> {
protected:
  virtual unsigned int getTagIdx(uint16_t tag);
};

template<uint16_t SZ>
TLV_ProtocolData::MemoryAllocator<SZ>::MemoryAllocator(uint16_t sz)
: maxBufferSize(0), posInBuf(0), externalBuff(false)
{
  setSize(sz);
}

template<uint16_t SZ>
TLV_ProtocolData::MemoryAllocator<SZ>::MemoryAllocator(MemoryAllocator& rhs)
{
  copyObject(rhs);
}

template<uint16_t SZ>
TLV_ProtocolData::MemoryAllocator<SZ>::~MemoryAllocator()
{
  if ( !isBufferOnStack && !externalBuff )
    delete [] allocation.bufferOnHeap;
}

template<uint16_t SZ>
TLV_ProtocolData::MemoryAllocator<SZ>&
TLV_ProtocolData::MemoryAllocator<SZ>::operator=(MemoryAllocator& rhs)
{
  if ( this != &rhs )
    copyObject(rhs);
  return *this;
}

template<uint16_t SZ>
void
TLV_ProtocolData::MemoryAllocator<SZ>::copyObject(MemoryAllocator& rhs)
{
  isBufferOnStack = rhs.isBufferOnStack;
  maxBufferSize = rhs.maxBufferSize;
  posInBuf = rhs.posInBuf;
  externalBuff = rhs.externalBuff;
  if ( isBufferOnStack )
    std::copy(rhs.allocation.bufferOnStack, rhs.allocation.bufferOnStack+posInBuf, allocation.bufferOnStack);
  else {
    allocation.bufferOnHeap = rhs.allocation.bufferOnHeap;
    rhs.allocation.bufferOnHeap = NULL;
  }
}

template<uint16_t SZ>
void
TLV_ProtocolData::MemoryAllocator<SZ>::append(const uint8_t* value, size_t sz)
{
  if ( posInBuf + sz > maxBufferSize )
    throw smsc::util::Exception("TLV_ProtocolData::MemoryAllocator::append::: buffer size exceeded, maxBufferSize=%u, posInBuf=%u, appended chunk size=%u",
                                maxBufferSize, posInBuf, sz);

  if ( externalBuff )
    throw smsc::util::Exception("TLV_ProtocolData::MemoryAllocator::append::: can't append to external buffer");

  if ( isBufferOnStack )
    memcpy(&allocation.bufferOnStack[posInBuf], value, sz);
  else
    memcpy(allocation.bufferOnHeap + posInBuf, value, sz);

  posInBuf += sz;
}

template<uint16_t SZ>
void
TLV_ProtocolData::MemoryAllocator<SZ>::setValue(const uint8_t* value, size_t sz)
{
  allocation.bufferOnHeap = const_cast<uint8_t*>(value); posInBuf = sz; maxBufferSize = sz;
  externalBuff = true; isBufferOnStack = false;
}

template<uint16_t SZ>
const uint8_t*
TLV_ProtocolData::MemoryAllocator<SZ>::get() const
{
  if ( isBufferOnStack )
    return allocation.bufferOnStack;
  else
    return allocation.bufferOnHeap;
}

template<uint16_t SZ>
uint16_t
TLV_ProtocolData::MemoryAllocator<SZ>::getCurrentSize() const
{
  return posInBuf;
}

template<uint16_t SZ>
void
TLV_ProtocolData::MemoryAllocator<SZ>::rewind()
{
  posInBuf = 0;
}

template<uint16_t SZ>
void
TLV_ProtocolData::MemoryAllocator<SZ>::setSize(uint16_t sz)
{
  if ( maxBufferSize ) {
    if ( maxBufferSize < sz ) {
      if ( !isBufferOnStack && !externalBuff )
        delete [] allocation.bufferOnHeap;
      allocation.bufferOnHeap = new uint8_t[sz];
      maxBufferSize = sz; isBufferOnStack = false;
    }
  } else {
    if ( sz > SZ ) {
      allocation.bufferOnHeap = new uint8_t[sz];
      isBufferOnStack = false;
      maxBufferSize = sz;
    } else {
      isBufferOnStack = true;
      maxBufferSize = SZ;
    }
  }
}

}}}}}}

#endif
