#ifndef __MCISME_ROLLINGFILESTORAGEADAPTER_HPP__
# define __MCISME_ROLLINGFILESTORAGEADAPTER_HPP__

# include "logger/Logger.h"
# include "mcisme/Storage.hpp"
# include "mcisme/DeliveryQueue.hpp"
# include "mcisme/RollingFileStorage.hpp"
# include "core/synchronization/Mutex.hpp"

namespace smsc {
namespace mcisme {

class RollingFileStorageAdapter : public Storage
{
public:
  RollingFileStorageAdapter()
  : _logger(smsc::logger::Logger::getInstance("mci.store")),
    _rollingFileStorage(NULL), _eventLifeTime(0)
  {}

  virtual ~RollingFileStorageAdapter();

  virtual int Init(smsc::util::config::ConfigView* storage_cfg_section, DeliveryQueue* delivery_queue);
  virtual int Init(const string& location, time_t event_life_time, uint8_t max_events, DeliveryQueue* delivery_queue);

  void init(const string& location, unsigned event_life_time, unsigned max_file_size,
            unsigned max_number_of_files, DeliveryQueue* delivery_queue);

  virtual void addEvent(const AbntAddr& called_num, const MCEvent& event, time_t sched_time);
  virtual void setSchedParams(const AbntAddr& called_num, time_t sched_time, uint16_t last_error = -1);
  virtual bool getEvents(const AbntAddr& called_num, vector<MCEvent>& events);
  virtual void deleteEvents(const AbntAddr& called_num, const vector<MCEvent>& events);

private:
  void loadEvents(DeliveryQueue* delivery_queue);
  bool killExpiredData(RecordData* record_data);
  void store_E_Event_in_logstore(const AbntAddrValue& called_abonent,
                                 const AbntAddrValue& calling_abonent);
  static const unsigned MAX_FILE_SIZE = 1024 * 1024;
  static const unsigned MAX_NUMBER_OF_FILES = 3;

  smsc::logger::Logger* _logger;
  RollingFileStorage* _rollingFileStorage;
  unsigned _eventLifeTime;
  core::synchronization::Mutex _lock;
};

}}

#endif
