#include "InfoSme_Id_Mapping_Entity_Adapter.hpp"

#include <util/crc32.h>

InfoSme_Id_Mapping_Entity_Adapter::InfoSme_Id_Mapping_Entity_Adapter()
  :  EntityStorable_Helper(0,0),
     _createdObj() {}
  
InfoSme_Id_Mapping_Entity_Adapter::InfoSme_Id_Mapping_Entity_Adapter(const InfoSme_Id_Mapping_Entity& rhs)
  : EntityStorable_Helper(0,0),
    _createdObj(rhs)
{
  size_t serialize_buf_sz = sizeof(uint64_t) + rhs.getSmscId().size() + rhs.getTaskId().size() + SERIALIZATION_BUF_OVERHEAD;

  smsc::util::SerializationBuffer buffer(serialize_buf_sz);

  buffer.WriteNetInt64(rhs.getId());
  buffer.WriteString<uint8_t>(rhs.getSmscId());
  buffer.WriteString<uint8_t>(rhs.getTaskId());

  _serialize_buf_size = buffer.getBufferSize();
  _serialize_buf = reinterpret_cast<uint8_t*>(buffer.releaseBuffer());
}

InfoSme_Id_Mapping_Entity_Adapter::InfoSme_Id_Mapping_Entity_Adapter(smsc::util::SerializationBuffer& inputBuf)
  : EntityStorable_Helper(reinterpret_cast<uint8_t*>(inputBuf.releaseBuffer()), inputBuf.getBufferSize()),
    _strict_field_order(inputBuf),
    _createdObj(_strict_field_order._id,
                _strict_field_order._smscId,
                _strict_field_order._taskId) {}

void
InfoSme_Id_Mapping_Entity_Adapter::marshal(smsc::util::SerializationBuffer* buf) const
{
  buf->Write(_serialize_buf, _serialize_buf_size);
}
