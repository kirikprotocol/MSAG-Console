#ifndef __DBENTITYSTORAGE_INFOSME_GENERATING_TASKS_ENTITY_ADAPTER_HPP__
# define __DBENTITYSTORAGE_INFOSME_GENERATING_TASKS_ENTITY_ADAPTER_HPP__ 1

# include <sys/types.h>
// next 3 includes for usage in couple with #include <util/BufferSerialization.hpp>
# include <netinet/in.h>
# include <inttypes.h>
# include <string.h>

# include <util/BufferSerialization.hpp>
# include "InfoSme_Generating_Tasks_Entity.hpp"
# include "EntityStorable_Helper.hpp"

class InfoSme_Generating_Tasks_Entity_Adapter : private EntityStorable_Helper {
public:
  InfoSme_Generating_Tasks_Entity_Adapter();
  InfoSme_Generating_Tasks_Entity_Adapter(const InfoSme_Generating_Tasks_Entity& rhs);
  
  explicit InfoSme_Generating_Tasks_Entity_Adapter(smsc::util::SerializationBuffer& inputBuf);

  void marshal(smsc::util::SerializationBuffer* buf) const;

  using EntityStorable_Helper::calcCrc;
  using EntityStorable_Helper::getSize;

  InfoSme_Generating_Tasks_Entity& getAdaptedObjRef() { return _createdObj; } // приветси к соответствию константность возвращаемых значений и объявлений методов
private:
  // вспомогательный класс для определения строго порядка вычисления аргументов
  // конструктора InfoSme_Generating_Tasks_Entity
  struct strict_field_order {
    strict_field_order() {}
    strict_field_order(smsc::util::SerializationBuffer& inputBuf) {
      inputBuf.ReadString<uint8_t>(_taskId);
    }
    std::string _taskId;
  };
  strict_field_order _strict_field_order;

  InfoSme_Generating_Tasks_Entity _createdObj;

  enum {SERIALIZATION_BUF_OVERHEAD = 1} overhead_size_t; // накладные расходы - 1 байт на длину поля taskId
};

#endif
