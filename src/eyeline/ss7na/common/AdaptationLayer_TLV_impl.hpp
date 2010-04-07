template <size_t SZ>
size_t
TLV_StringPrimitive<SZ>::serialize(TP* packet_buf,
                                   size_t offset /*position inside buffer where TLV object will be stored*/) const
{
  if ( !_isValueSet )
    throw utilx::FieldNotSetException("TLV_StringPrimitive::serialize::: value isn't set");

  offset = AdaptationLayer_TLV::serialize(packet_buf, offset);
  return addField(packet_buf, offset, reinterpret_cast<const uint8_t*>(_value), _valLen);
}

template <size_t SZ>
size_t
TLV_StringPrimitive<SZ>::deserialize(const TP& packet_buf,
                                     size_t offset /*position inside buffer where tag's data started*/,
                                     uint16_t val_len)
{
  if ( val_len > sizeof(_value) )
    throw smsc::util::Exception("TLV_StringPrimitive::deserialize::: value length [=%d] exceeded max. allowable value [=%d]", val_len, SZ);

  offset = extractField(packet_buf, offset, reinterpret_cast<uint8_t*>(_value), val_len);
  _valLen = val_len; _isValueSet = true;
  return offset;
}

template <size_t SZ>
int
TLV_StringPrimitive<SZ>::getUtf8Len(uint8_t first_utf8_octet) const
{
  if ( first_utf8_octet & 0x7f == first_utf8_octet ) return 1;
  uint8_t mask = 0x80;
  int i = 0;
  while ( (mask & first_utf8_octet) && i++ < 5) mask >>= 1;
  return i;
}

template <size_t SZ>
std::string
TLV_StringPrimitive<SZ>::getPrintableValue() const
{
  if ( !_isValueSet )
    throw utilx::FieldNotSetException("TLV_StringPrimitive::getValue::: value isn't set");

  std::string result;

  for (int i=0; i<_valLen;) {
    int nextSymbolLen = getUtf8Len(_value[i]);
    if ( nextSymbolLen == 1 ) {
      result += _value[i++];
    } else {
      char hexBuf[4];
      while ( nextSymbolLen-- > 0 ) {
        sprintf(hexBuf, "=%02X", uint8_t(_value[i++]));
        result.append(hexBuf);
      }
    }
  }
  
  return result;
}

template <size_t SZ>
bool
TLV_StringPrimitive<SZ>::isSetValue() const { return _isValueSet; }

template <size_t SZ>
uint16_t
TLV_StringPrimitive<SZ>::getLength() const
{
  return HEADER_SZ + _valLen;
}

template <size_t SZ>
TLV_OctetArrayPrimitive<SZ>::TLV_OctetArrayPrimitive(uint16_t tag)
  : AdaptationLayer_TLV(tag), _valLen(0), _isValueSet(false), _paddingLen(0)
{}

template <size_t SZ>
TLV_OctetArrayPrimitive<SZ>::TLV_OctetArrayPrimitive(const TLV_OctetArrayPrimitive<SZ>& rhs)
  : AdaptationLayer_TLV(rhs), _valLen(rhs._valLen), _isValueSet(rhs._isValueSet), _paddingLen(rhs._paddingLen)
{
  if(_isValueSet)
  {
    memcpy(_valueBuffer._value,rhs._valueBuffer._value,rhs._valLen);
  }
}

template <size_t SZ>
TLV_OctetArrayPrimitive<SZ>::TLV_OctetArrayPrimitive(uint16_t tag, const uint8_t* val, uint16_t val_len)
  : AdaptationLayer_TLV(tag), _valLen(val_len), _isValueSet(true), _paddingLen(0)
{
  if ( _valLen > sizeof(_valueBuffer) )
    throw smsc::util::Exception("TLV_OctetArrayPrimitive::TLV_OctetArrayPrimitive::: argument size [%d] is too large [max size=%d]", _valLen, sizeof(_valueBuffer));

  memcpy(_valueBuffer._value, val, _valLen);
  int paddingTo4bytes = _valLen & 0x03;
  if ( paddingTo4bytes )
    _paddingLen = 0x04 - paddingTo4bytes;
}

template <size_t SZ>
TLV_OctetArrayPrimitive<SZ>::TLV_OctetArrayPrimitive(uint16_t tag, size_t reserved_octets_offset, const uint8_t* val, uint16_t val_len)
  : AdaptationLayer_TLV(tag),
    _valLen(static_cast<uint16_t>(reserved_octets_offset)+val_len), _isValueSet(true), _paddingLen(0)
{
  if ( _valLen > sizeof(_valueBuffer) )
    throw smsc::util::Exception("TLV_OctetArrayPrimitive::TLV_OctetArrayPrimitive::: argument size [%d] is too large [max size=%d]", _valLen, sizeof(_valueBuffer));
  else {
    memset(_valueBuffer._value, 0, reserved_octets_offset);
    memcpy(_valueBuffer._value + reserved_octets_offset, val, val_len);

    int paddingTo4bytes = _valLen & 0x03;
    if ( paddingTo4bytes )
      _paddingLen = 0x04 - paddingTo4bytes;
  }
}

template <size_t SZ>
TLV_OctetArrayPrimitive<SZ>& TLV_OctetArrayPrimitive<SZ>::operator=(const TLV_OctetArrayPrimitive<SZ>& rhs)
{
  _tag=rhs._tag;
  _isValueSet=rhs._isValueSet;
  _valLen=rhs._valLen;
  _paddingLen=rhs._paddingLen;
  if(_isValueSet)
  {
    memcpy(_valueBuffer._value,rhs._valueBuffer._value,_valLen);
  }
  return *this;
}

template <size_t SZ>
size_t
TLV_OctetArrayPrimitive<SZ>::serialize(TP* packet_buf,
                                       size_t offset) const
{
  if ( !_isValueSet )
    throw utilx::FieldNotSetException("TLV_OctetPrimitive::serialize::: value isn't set");

  offset = AdaptationLayer_TLV::serialize(packet_buf, offset);
  offset = addField(packet_buf, offset, _valueBuffer._value, _valLen);

  if ( _paddingLen ) {
    uint8_t padding[4]={0};
    offset = addField(packet_buf, offset, padding, _paddingLen);
  }
  return offset;
}

template <size_t SZ>
size_t
TLV_OctetArrayPrimitive<SZ>::deserialize(const TP& packet_buf,
                                         size_t offset,
                                         uint16_t val_len)
{
  if ( val_len > sizeof(_valueBuffer) )
    throw smsc::util::Exception("TLV_OctetArrayPrimitive::deserialize::: value length [=%d] exceeded max. allowable value [=%d]", val_len, sizeof(_valueBuffer));

  offset = extractField(packet_buf, offset, _valueBuffer._value, val_len);

  int paddingTo4bytes = static_cast<int>(offset & 0x03);
  if ( paddingTo4bytes ) {
    _paddingLen = 0x04 - paddingTo4bytes;
    offset += _paddingLen;
  }
  _valLen = val_len; _isValueSet = true;

  return offset;
}

template <size_t SZ>
const uint8_t*
TLV_OctetArrayPrimitive<SZ>::getValue() const {
  if ( !_isValueSet )
    throw utilx::FieldNotSetException("TLV_OctetArrayPrimitive::getValue::: value isn't set");

  return _valueBuffer._value;
}

template <size_t SZ>
bool
TLV_OctetArrayPrimitive<SZ>::isSetValue() const { return _isValueSet; }

template <size_t SZ>
uint16_t
TLV_OctetArrayPrimitive<SZ>::getLength() const
{
  return HEADER_SZ + _valLen + _paddingLen;
}

template <size_t SZ>
uint16_t
TLV_OctetArrayPrimitive<SZ>::getValueLength() const
{
  return _valLen;
}

template <size_t SZ>
uint16_t
TLV_OctetArrayPrimitive<SZ>::getActualLength() const
{
  return HEADER_SZ + _valLen;
}

template <size_t NUM_OF_TAGS>
AdaptationLayer_TLV_Factory<NUM_OF_TAGS>::AdaptationLayer_TLV_Factory()
  : _numsOfMandatoryTlvObjects(0)
{
  memset(reinterpret_cast<uint8_t*>(_optionalTlvTypeToTLVObject), 0, sizeof(_optionalTlvTypeToTLVObject));
  memset(reinterpret_cast<uint8_t*>(_mandatoryTlvTypeToTLVObject), 0, sizeof(_mandatoryTlvTypeToTLVObject));
}

template <size_t NUM_OF_TAGS>
void
AdaptationLayer_TLV_Factory<NUM_OF_TAGS>::registerExpectedOptionalTlv(AdaptationLayer_TLV* expected_tlv)
{
  _optionalTlvTypeToTLVObject[getTagIdx(expected_tlv->getTag())] = expected_tlv;
}

template <size_t NUM_OF_TAGS>
void
AdaptationLayer_TLV_Factory<NUM_OF_TAGS>::registerExpectedMandatoryTlv(AdaptationLayer_TLV* expected_tlv)
{
  _mandatoryTlvTypeToTLVObject[getTagIdx(expected_tlv->getTag())] = expected_tlv;
  ++_numsOfMandatoryTlvObjects;
}

template <size_t NUM_OF_TAGS>
void
AdaptationLayer_TLV_Factory<NUM_OF_TAGS>::setPositionTo4BytesBoundary(size_t* offset)
{
  size_t paddingTo4bytes = static_cast<int>(*offset & 0x03);
  if ( paddingTo4bytes ) // if tag length is not a multiple of 4 bytes
    *offset += 0x04 - paddingTo4bytes;
}

template <size_t NUM_OF_TAGS>
void
AdaptationLayer_TLV_Factory<NUM_OF_TAGS>::generateProtocolException()
{
  AdaptationLayer_TLV* tlvObj;
  int i=0, offset=0;
  char bufForTagsList[NUM_OF_TAGS*7+1]; // 7 bytes (including space for comma) for each tag presentation
  while (_numsOfMandatoryTlvObjects-- > 0) {
    while (i<NUM_OF_TAGS) {
      if ( (tlvObj = _mandatoryTlvTypeToTLVObject[i++] ) ) {
        offset=sprintf(bufForTagsList+offset, "0x%04X,",tlvObj->getTag()); break;
      }
    }
  }
  bufForTagsList[strlen(bufForTagsList)-1]=0;
  throw io_dispatcher::ProtocolException("AdaptationLayer_TLV_Factory::generateProtocolException::: missing mandatory tlv(s) with next tag values [%s] when parsing input buffer", bufForTagsList);
}

template <size_t NUM_OF_TAGS>
size_t
AdaptationLayer_TLV_Factory<NUM_OF_TAGS>::parseInputBuffer(const common::TP& packet_buf, size_t offset)
{
  uint16_t tag, valLen;

  while ( offset < packet_buf.packetLen ) {
    setPositionTo4BytesBoundary(&offset);

    if ( offset >= packet_buf.packetLen ) break;

    offset = extractField(packet_buf, offset, &tag);
    offset = extractField(packet_buf, offset, &valLen);

    valLen -= static_cast<uint16_t>(sizeof(tag) + sizeof(valLen));

    AdaptationLayer_TLV* suaTLVObject = NULL;
    if ( _numsOfMandatoryTlvObjects ) {
      suaTLVObject = _mandatoryTlvTypeToTLVObject[getTagIdx(tag)];
      if ( suaTLVObject ) {
        offset = suaTLVObject->deserialize(packet_buf, offset, valLen);
        _mandatoryTlvTypeToTLVObject[getTagIdx(tag)] = NULL;
        --_numsOfMandatoryTlvObjects; continue;
      }
    }
    suaTLVObject = _optionalTlvTypeToTLVObject[getTagIdx(tag)];
    if ( suaTLVObject ) {
      offset = suaTLVObject->deserialize(packet_buf, offset, valLen);
      _optionalTlvTypeToTLVObject[getTagIdx(tag)] = NULL;
    } else throw io_dispatcher::ProtocolException("AdaptationLayer_TLV_Factory::parseInputBuffer::: unexpected or duplicate tlv with tag value [=0x%04X]", tag);

  }

  if ( _numsOfMandatoryTlvObjects )
    generateProtocolException();

  return offset;
}
