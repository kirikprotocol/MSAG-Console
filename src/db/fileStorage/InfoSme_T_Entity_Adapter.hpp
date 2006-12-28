#ifndef ___DBENTITYSTORAGE_INFOSME_T_ENTITY_ADAPTER_HPP__
# define ___DBENTITYSTORAGE_INFOSME_T_ENTITY_ADAPTER_HPP__ 1

# include <sys/types.h>
// next 3 includes for usage in couple with #include <util/BufferSerialization.hpp>
# include <netinet/in.h>
# include <inttypes.h>
# include <string.h>

# include <util/BufferSerialization.hpp>
# include "InfoSme_T_Entity.hpp"
# include "EntityStorable_Helper.hpp"

class InfoSme_T_Entity_Adapter : private EntityStorable_Helper {
public:
  InfoSme_T_Entity_Adapter();
  InfoSme_T_Entity_Adapter(const InfoSme_T_Entity& rhs);

  // interface expected by DataStorage_FileDispatcher class
  explicit InfoSme_T_Entity_Adapter(smsc::util::SerializationBuffer& inputBuf);

  void marshal(smsc::util::SerializationBuffer* buf) const;

  using EntityStorable_Helper::calcCrc;
  using EntityStorable_Helper::getSize;

  InfoSme_T_Entity& getAdaptedObjRef() { return _createdObj; } // приветси к соответствию константность возвращаемых значений и объявлений методов

private:
  // вспомогательный класс для определения строго порядка вычисления аргументов
  // конструктора InfoSme_T_Entity
  struct strict_field_order {
    strict_field_order() {}
    strict_field_order(smsc::util::SerializationBuffer& inputBuf) {
      _id = inputBuf.ReadNetInt64();
      _state = inputBuf.ReadByte();
      inputBuf.ReadString<uint8_t>(_abonent);
#ifdef B64
      _sendDate = inputBuf.ReadNetInt64();
#else
      _sendDate = inputBuf.ReadNetInt32();
#endif
      inputBuf.ReadString<uint16_t>(_msg);
    }
    uint64_t _id;
    uint8_t _state;
    std::string _abonent;
    time_t _sendDate;
    std::string _msg;
  };
  strict_field_order _strict_field_order;

  InfoSme_T_Entity _createdObj;

  enum {SERIALIZATION_BUF_OVERHEAD = 3} overhead_size_t; // два байта на длину сообщения и байт на длину адреса
};

#endif
