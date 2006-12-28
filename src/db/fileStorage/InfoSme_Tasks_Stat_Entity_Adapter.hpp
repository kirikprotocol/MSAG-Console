#ifndef __DBENTITYSTORAGE_INFOSME_TASKS_STAT_ENTITY_ADAPTER_HPP__
# define __DBENTITYSTORAGE_INFOSME_TASKS_STAT_ENTITY_ADAPTER_HPP__ 1

# include <sys/types.h>
// next 3 includes for usage in couple with #include <util/BufferSerialization.hpp>
# include <netinet/in.h>
# include <inttypes.h>
# include <string.h>

# include <util/BufferSerialization.hpp>
# include "InfoSme_Tasks_Stat_Entity.hpp"
# include "EntityStorable_Helper.hpp"

class InfoSme_Tasks_Stat_Entity_Adapter : private EntityStorable_Helper {
public:
  InfoSme_Tasks_Stat_Entity_Adapter();
  InfoSme_Tasks_Stat_Entity_Adapter(const InfoSme_Tasks_Stat_Entity& rhs);

  explicit InfoSme_Tasks_Stat_Entity_Adapter(smsc::util::SerializationBuffer& inputBuf);

  void marshal(smsc::util::SerializationBuffer* buf) const;

  using EntityStorable_Helper::calcCrc;
  using EntityStorable_Helper::getSize;

  InfoSme_Tasks_Stat_Entity& getAdaptedObjRef() { return _createdObj; } // приветси к соответствию константность возвращаемых значений и объявлений методов

private:
  // вспомогательный класс для определения строго порядка вычисления аргументов
  // конструктора InfoSme_Generating_Tasks_Entity
  struct strict_field_order {
    strict_field_order() {}
    strict_field_order(smsc::util::SerializationBuffer& inputBuf) {
      inputBuf.ReadString<uint8_t>(_taskId);
      _period = inputBuf.ReadNetInt32();
      _generated = inputBuf.ReadNetInt32();
      _delivered = inputBuf.ReadNetInt32();
      _retried = inputBuf.ReadNetInt32();
      _failed = inputBuf.ReadNetInt32();
    }
    std::string _taskId;
    uint32_t _period;
    uint32_t _generated;
    uint32_t _delivered;
    uint32_t _retried;
    uint32_t _failed;
  };
  strict_field_order _strict_field_order;

  InfoSme_Tasks_Stat_Entity _createdObj;

  enum {SERIALIZATION_BUF_OVERHEAD = 1} overhead_size_t; // накладные расходы - 1 байт на длину поля taskId
};

#endif
