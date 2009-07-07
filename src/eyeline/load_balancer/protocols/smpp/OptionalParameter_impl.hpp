template <class T, uint16_t TAG>
uint16_t
OptionalParameter<T, TAG>::getTag() const
{
  return TAG;
}

template <class T, uint16_t TAG>
T
OptionalParameter<T, TAG>::getValue() const
{
  if ( !_isSetValue )
    throw utilx::FieldNotSetException("OptionalParameter::getValue::: value with TAG=%04X is not set", TAG);
  return _value;
}

template <class T, uint16_t TAG>
bool
OptionalParameter<T, TAG>::isSetValue() const
{
  return _isSetValue;
}

template <class T, uint16_t TAG>
void
OptionalParameter<T, TAG>::setValue(T value)
{
  _value = value; 
  _isSetValue = true;
}

template <class T, uint16_t TAG>
size_t
OptionalParameter<T, TAG>::serialize(io_subsystem::Packet* packet) const
{
  if ( !_isSetValue )
    throw utilx::FieldNotSetException("OptionalParameter::serialize::: value with TAG=%04X is not set", TAG);

  packet->addValue(TAG);
  packet->addValue(static_cast<uint16_t>(sizeof(_value)));
  return packet->addValue(_value);
}

template <class T, uint16_t TAG>
size_t
OptionalParameter<T, TAG>::deserialize(const io_subsystem::Packet* packet, size_t offset)
{
  uint16_t tag;
  if ( !packet->isSufficientSpace(offset, _MINIMAL_PARAMETER_LENGTH) )
    return offset;

  offset = packet->extractValue(&tag, offset);
  if ( tag != TAG )
    throw utilx::DeserializationException("OptionalParameter::deserialize::: got unexpected tag value=0x%04X, expected tag value=0x%04X", tag, TAG);
  uint16_t valueLength;
  offset = packet->extractValue(&valueLength, offset);
  if ( valueLength != sizeof(T) )
    throw utilx::DeserializationException("OptionalParameter::deserialize::: got wrong length of value=%d, expected length=%d", valueLength, sizeof(T));

  offset = packet->extractValue(&_value, offset);
  _isSetValue = true;

  return offset;
}

template <class T, uint16_t TAG>
uint32_t
OptionalParameter<T, TAG>::getParameterSize() const
{
  return static_cast<uint32_t>(sizeof(TAG) + _LEN_SIZE + sizeof(T));
}
