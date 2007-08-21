#include "InfoSme_T_Entity_Adapter.hpp"

InfoSme_T_Entity_Adapter::InfoSme_T_Entity_Adapter()
  : EntityStorable_Helper(0,0),
    _createdObj() {}

InfoSme_T_Entity_Adapter::InfoSme_T_Entity_Adapter(const InfoSme_T_Entity& rhs)
  : EntityStorable_Helper(0,0),
    _createdObj(rhs)
{
  size_t serialize_buf_sz = sizeof(uint64_t) + sizeof(uint8_t) + rhs.getAbonentAddress().size() + sizeof(time_t) + rhs.getMessage().size() + SERIALIZATION_BUF_OVERHEAD;

  smsc::util::SerializationBuffer buffer(serialize_buf_sz);

  buffer.WriteNetInt64(rhs.getId());
  buffer.WriteByte(rhs.getState());
  buffer.WriteString<uint8_t>(rhs.getAbonentAddress());
#ifdef B64
  buffer.WriteNetInt64(rhs.getSendDate());
#else
  buffer.WriteNetInt32(rhs.getSendDate());
#endif
  buffer.WriteString<uint16_t>(rhs.getMessage());
  buffer.WriteString<uint8_t>(rhs.getRegionId());

  _serialize_buf_size = buffer.getBufferSize();
  _serialize_buf = reinterpret_cast<uint8_t*>(buffer.releaseBuffer());
}

InfoSme_T_Entity_Adapter::InfoSme_T_Entity_Adapter(smsc::util::SerializationBuffer& inputBuf)
  : EntityStorable_Helper(reinterpret_cast<uint8_t*>(inputBuf.releaseBuffer()), inputBuf.getBufferSize()),
    _strict_field_order(inputBuf),
    _createdObj(_strict_field_order._id,
                _strict_field_order._state,
                _strict_field_order._abonent,
                _strict_field_order._sendDate,
                _strict_field_order._msg,
                _strict_field_order._regionId) {}

void
InfoSme_T_Entity_Adapter::marshal(smsc::util::SerializationBuffer* buf) const
{
  buf->Write(_serialize_buf, _serialize_buf_size);
}
