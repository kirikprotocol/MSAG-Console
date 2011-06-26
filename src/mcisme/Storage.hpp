//------------------------------------
//  Storage.hpp
//  Routman Michael, 2005-2006
//------------------------------------
//
//	Файл содержит описание интерфейсного класса Storage.
//

#ifndef ___STORAGE_H
#define ___STORAGE_H

#include <vector>
#include <string>

#include "core/buffers/Array.hpp"
#include "mcisme/AbntAddr.hpp"
#include "mcisme/DeliveryQueue.hpp"
#include "mcisme/Messages.h"

#include "util/config/ConfigView.h"
#include "util/config/ConfigException.h"

namespace smsc {
namespace mcisme {

using std::vector;
using std::string;
//using core::buffers::Array;
//using namespace sms;

//	Register New Event Policy
const uint8_t REJECT_NEW_EVENT = 0x01;
const uint8_t DISPLACE_OLDEST_EVENT = 0x02;

class Storage
{
public:
  Storage(){}
  virtual ~Storage(){}
  virtual int Init(smsc::util::config::ConfigView* storageConfig, DeliveryQueue* pDeliveryQueue) = 0;
  virtual int Init(const string& location, time_t eventLifeTime, uint8_t maxEvents, DeliveryQueue* pDeliveryQueue) = 0;
  virtual void addEvent(const AbntAddr& CalledNum, const MCEvent& event, time_t schedTime) = 0;
  virtual void setSchedParams(const AbntAddr& CalledNum, time_t schedTime, uint16_t lastError = -1) = 0;
  virtual bool getEvents(const AbntAddr& CalledNum, vector<MCEvent>& events) = 0;
  virtual void deleteEvents(const AbntAddr& CalledNum, const vector<MCEvent>& events) = 0;
};


};	//  namespace mcisme
};	//  namespace smsc
#endif
