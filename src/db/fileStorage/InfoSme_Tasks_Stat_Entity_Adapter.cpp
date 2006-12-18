#include "InfoSme_Tasks_Stat_Entity_Adapter.hpp"

#include <util/crc32.h>

InfoSme_Tasks_Stat_Entity_Adapter::InfoSme_Tasks_Stat_Entity_Adapter()
  : EntityStorable_Helper(0,0),
    _createdObj() {}

InfoSme_Tasks_Stat_Entity_Adapter::InfoSme_Tasks_Stat_Entity_Adapter(const InfoSme_Tasks_Stat_Entity& rhs)
  : EntityStorable_Helper(0,0),
    _createdObj(rhs)
{
  size_t serialize_buf_sz = rhs.getTaskId().size() + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint32_t) + SERIALIZATION_BUF_OVERHEAD;

  smsc::util::SerializationBuffer buffer(serialize_buf_sz);

  buffer.WriteString<uint8_t>(rhs.getTaskId());
  buffer.WriteNetInt32(rhs.getPeriod());
  buffer.WriteNetInt32(rhs.getGenerated());
  buffer.WriteNetInt32(rhs.getDelivered());
  buffer.WriteNetInt32(rhs.getRetried());
  buffer.WriteNetInt32(rhs.getFailed());

  _serialize_buf_size = buffer.getBufferSize();
  _serialize_buf = reinterpret_cast<uint8_t*>(buffer.releaseBuffer());
}

InfoSme_Tasks_Stat_Entity_Adapter::InfoSme_Tasks_Stat_Entity_Adapter(smsc::util::SerializationBuffer& inputBuf)
  : EntityStorable_Helper(reinterpret_cast<uint8_t*>(inputBuf.releaseBuffer()), inputBuf.getBufferSize()),
    _strict_field_order(inputBuf),
    _createdObj(_strict_field_order._taskId,
                _strict_field_order._period,
                _strict_field_order._generated,
                _strict_field_order._delivered,
                _strict_field_order._retried,
                _strict_field_order._failed) {}

void
InfoSme_Tasks_Stat_Entity_Adapter::marshal(smsc::util::SerializationBuffer* buf) const
{
  buf->Write(_serialize_buf, _serialize_buf_size);
}
