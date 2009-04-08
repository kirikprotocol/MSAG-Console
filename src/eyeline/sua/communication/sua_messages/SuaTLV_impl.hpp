template <size_t SZ>
size_t
TLV_StringPrimitive<SZ>::serialize(communication::TP* packetBuf,
                                   size_t offset /*position inside buffer where TLV object will be stored*/) const
{
  if ( !_isValueSet )
    throw utilx::FieldNotSetException("TLV_StringPrimitive::serialize::: value isn't set");

  offset = SuaTLV::serialize(packetBuf, offset);
  return communication::addField(packetBuf, offset, reinterpret_cast<const uint8_t*>(_value), _valLen);
}

template <size_t SZ>
size_t
TLV_StringPrimitive<SZ>::deserialize(const communication::TP& packetBuf,
                                     size_t offset /*position inside buffer where tag's data started*/,
                                     uint16_t valLen)
{
  if ( valLen > sizeof(_value) )
    throw smsc::util::Exception("TLV_StringPrimitive::deserialize::: value length [=%d] exceeded max. allowable value [=%d]", valLen, SZ);

  offset = communication::extractField(packetBuf, offset, reinterpret_cast<uint8_t*>(_value), valLen);
  _valLen = valLen; _isValueSet = true;
  return offset;
}

template <size_t SZ>
int
TLV_StringPrimitive<SZ>::getUtf8Len(uint8_t firstUtf8Octet) const
{
  if ( firstUtf8Octet & 0x7f == firstUtf8Octet ) return 1;
  uint8_t mask = 0x80;
  int i = 0;
  while ( (mask & firstUtf8Octet) && i++ < 5) mask >>= 1;
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
  : SuaTLV(tag), _valLen(0), _isValueSet(false), _paddingLen(0)
{
  //memset(_valueBuffer._value, 0, sizeof(_valueBuffer));
}

TLV_OctetArrayPrimitive::TLV_OctetArrayPrimitive(const TLV_OctetArrayPrimitive& rhs)
  : SuaTLV(rhs), _valLen(rhs._valLen), _isValueSet(rhs._isValueSet), _paddingLen(rhs._paddingLen)
{
  if(_isValueSet)
  {
    memcpy(_valueBuffer._value,rhs._valueBuffer._value,rhs._valLen);
  }
}


template <size_t SZ>
TLV_OctetArrayPrimitive<SZ>::TLV_OctetArrayPrimitive(uint16_t tag, const uint8_t* val, uint16_t valLen)
  : SuaTLV(tag), _valLen(valLen), _isValueSet(true), _paddingLen(0)
{
  if ( _valLen > sizeof(_valueBuffer) )
    throw smsc::util::Exception("TLV_OctetArrayPrimitive::TLV_OctetArrayPrimitive::: argument size [%d] is too large [max size=%d]", _valLen, sizeof(_valueBuffer));

  memcpy(_valueBuffer._value, val, _valLen);
  int paddingTo4bytes = _valLen & 0x03;
  if ( paddingTo4bytes )
    _paddingLen = 0x04 - paddingTo4bytes;
}

template <size_t SZ>
TLV_OctetArrayPrimitive<SZ>::TLV_OctetArrayPrimitive(uint16_t tag, size_t reservedOctetsOffset, const uint8_t* val, uint16_t valLen)
  : SuaTLV(tag), _valLen(static_cast<uint16_t>(reservedOctetsOffset)+valLen), _isValueSet(true), _paddingLen(0)
{
  if ( _valLen > sizeof(_valueBuffer) )
    throw smsc::util::Exception("TLV_OctetArrayPrimitive::TLV_OctetArrayPrimitive::: argument size [%d] is too large [max size=%d]", _valLen, sizeof(_valueBuffer));
  else {
    memset(_valueBuffer._value, 0, reservedOctetsOffset);
    memcpy(_valueBuffer._value + reservedOctetsOffset, val, valLen);

    int paddingTo4bytes = _valLen & 0x03;
    if ( paddingTo4bytes )
      _paddingLen = 0x04 - paddingTo4bytes;
  }
}

template <size_t SZ>
size_t
TLV_OctetArrayPrimitive<SZ>::serialize(communication::TP* packetBuf,
                                       size_t offset /*position inside buffer where TLV object will be stored*/) const
{
  if ( !_isValueSet )
    throw utilx::FieldNotSetException("TLV_StringPrimitive::serialize::: value isn't set");

  offset = SuaTLV::serialize(packetBuf, offset);
  offset = communication::addField(packetBuf, offset, _valueBuffer._value, _valLen);

  if ( _paddingLen ) {
    uint8_t padding[4]={0};
    offset = communication::addField(packetBuf, offset, padding, _paddingLen);
  }
  return offset;
}

template <size_t SZ>
size_t
TLV_OctetArrayPrimitive<SZ>::deserialize(const communication::TP& packetBuf,
                                         size_t offset /*position inside buffer where tag's data started*/,
                                         uint16_t valLen)
{
  if ( valLen > sizeof(_valueBuffer) )
    throw smsc::util::Exception("TLV_OctetArrayPrimitive::deserialize::: value length [=%d] exceeded max. allowable value [=%d]", valLen, sizeof(_valueBuffer));

  offset = communication::extractField(packetBuf, offset, _valueBuffer._value, valLen);

  int paddingTo4bytes = static_cast<int>(offset & 0x03);
  if ( paddingTo4bytes ) {
    _paddingLen = 0x04 - paddingTo4bytes;
    offset += _paddingLen;
  }
  _valLen = valLen; _isValueSet = true;

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
