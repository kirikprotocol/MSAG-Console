//------------------------------------
//  DeliveryQueue.hpp
//  Routman Michael, 2005-2006
//------------------------------------


#ifndef ___DELIVERYQUEUE_H
#define ___DELIVERYQUEUE_H

#include <map>
#include <vector>
#include <sys/types.h>
#include <time.h>

#include <logger/Logger.h>

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/synchronization/EventMonitor.hpp>

#include <core/buffers/Hash.hpp>

#include <mcisme/AbntAddr.hpp>
#include <system/status.h>

namespace smsc { namespace mcisme
{

using std::multimap;
using std::map;
using std::pair;
using std::vector;
using namespace core::synchronization;
using namespace core::buffers;

typedef uint8_t abnt_stat_t;

const abnt_stat_t	Idle			= 0x00;
const abnt_stat_t	InProcess		= 0x01;
const abnt_stat_t	AlertHandled	= 0x02;

const time_t	default_wait = 60;

struct SchedItem
{
	time_t		schedTime;
	
	AbntAddr	abnt;
	uint8_t		eventsCount;
	uint32_t	lastError;

	uint32_t	abonentsCount;
	
	SchedItem(){}
	SchedItem(time_t t, const AbntAddr& _abnt, uint8_t ec, uint32_t le, uint32_t ac):
		schedTime(t), abnt(_abnt), eventsCount(ec), lastError(le), abonentsCount(ac){}
	SchedItem(const SchedItem& item):
		schedTime(item.schedTime), abnt(item.abnt), eventsCount(item.eventsCount), lastError(item.lastError), abonentsCount(item.abonentsCount){}
	SchedItem& operator=(const SchedItem& item)
	{
		if(this != &item)
		{
			schedTime = item.schedTime;
			abnt = item.abnt;
			eventsCount = item.eventsCount;
			lastError = item.lastError;
			abonentsCount = item.abonentsCount;
		}
		return *this;
	}
	bool operator<(const SchedItem& item) const
	{return schedTime < item.schedTime;}
};

struct SchedParam
{
	abnt_stat_t abntStatus;
//	int8_t		count;
	time_t		schedTime;
	uint16_t	lastError;
};

char* cTime(const time_t* clock);		// функция возвращает на статический буфер. Не потокобезопасная.

class DeliveryQueue
{
	typedef multimap<time_t, AbntAddr>	DelQueue;
	typedef map<int, time_t>			SchedTable;
	typedef DelQueue::iterator	DelQueueIter;

	DelQueue			deliveryQueue;
	SchedTable			scheduleTable;
	time_t				schedTimeOnBusy;
	EventMonitor		deliveryQueueMonitor;
	Hash<SchedParam>	AbntsStatus;
//	Hash<abnt_stat_t>	AbntsStatus;
		
	time_t				dt;
	uint32_t			total;
	bool				isQueueOpen;

	smsc::logger::Logger *logger;

public:

	DeliveryQueue(time_t _dt = 5, time_t onBusy = 300):
		dt(_dt), schedTimeOnBusy(onBusy), total(0), isQueueOpen(false),
		logger(smsc::logger::Logger::getInstance("mci.DlvQueue")){}
	DeliveryQueue(const DeliveryQueue& addr){}
	virtual ~DeliveryQueue(){Erase();}

	void AddScheduleRow(int error, time_t wait);
	void SetSchedTimeOnBusy(time_t wait);
	
	void OpenQueue(void);
	void CloseQueue(void);
	bool isQueueOpened(void);
	int GetAbntCount(void);
	int GetQueueSize(void);

	time_t Schedule(const AbntAddr& abnt, bool onBusy=false, time_t schedTime=-1, uint16_t lastError=-1);
	time_t Reschedule(const AbntAddr& abnt, int resp_status = smsc::system::Status::OK); // bool toHead = false
	time_t RegisterAlert(const AbntAddr& abnt);
	bool Get(AbntAddr& abnt);
	bool Get(const AbntAddr& abnt, SchedItem& item);
//	int Get(vector<SchedItem>& items, int count);
	int Get(vector<SchedItem>& items, int count);
	void Remove(const AbntAddr& abnt);
	void Erase(void);

private:
	
	void Resched(const AbntAddr& abnt, time_t oldSchedTime, time_t newSchedTime);
	time_t CalcTimeDelivery(int err = -1);
	time_t GetDeliveryTime(void);

};

};	//  namespace msisme
};	//  namespace smsc
#endif


//struct SchedItem
//{
//	time_t		time;
//	AbntAddr	abnt;
//	
//	SchedItem(){}
//	SchedItem(const time_t& t, const AbntAddr& _abnt):time(t), abnt(_abnt){}
//	SchedItem(const SchedItem& item):time(item.time), abnt(item.abnt){}
//	SchedItem& operator=(const SchedItem& item)
//	{
//		if(this != &item)
//		{
//			time = item.time;
//			abnt = item.abnt;
//		}
//		return *this;
//	}
//	bool operator<(const SchedItem& item) const
//	{return time < item.time;}
//};
//
//struct SchedParam
//{
//	abnt_stat_t abntStatus;
//	int8_t		count;
//};
//
//class DeliveryQueue
//{
//	multimap<time_t, AbntAddr>		deliveryQueue;
//	map<int, time_t>	scheduleTable;
//	time_t				schedTimeOnBusy;
//	EventMonitor		deliveryQueueMonitor;
//	Hash<SchedParam>	AbntsStatus;
////	Hash<abnt_stat_t>	AbntsStatus;
//		
//	time_t				dt;
//	uint32_t			total;
//	bool				isQueueOpen;
//
//	smsc::logger::Logger *logger;
//
//public:
//
//	DeliveryQueue(time_t _dt = 5, time_t onBusy = 300):
//		dt(_dt), schedTimeOnBusy(onBusy), total(0), isQueueOpen(false),
//		logger(smsc::logger::Logger::getInstance("mci.DlvQueue")){}
//	DeliveryQueue(const DeliveryQueue& addr){}
//	virtual ~DeliveryQueue(){Erase();}
//
//	void AddScheduleRow(int error, time_t wait)
//	{
//		MutexGuard lock(deliveryQueueMonitor);
//		if(error == -1)
//			dt = wait;
//		else
//			scheduleTable.insert(multimap<int, time_t>::value_type(error, wait));
//	}
//	void SetSchedTimeOnBusy(time_t wait)
//	{
//		MutexGuard lock(deliveryQueueMonitor);
//		schedTimeOnBusy = wait;
//	}
//
//	void OpenQueue(void)
//	{
//		MutexGuard lock(deliveryQueueMonitor);
//		isQueueOpen = true;
//		deliveryQueueMonitor.notify();
//	}
//
//	void CloseQueue(void)
//	{
////		MutexGuard lock(deliveryQueueMonitor);
//		isQueueOpen = false;
////		deliveryQueueMonitor.notify();
//	}
//
//	bool isQueueOpened(void)
//	{
////		MutexGuard lock(deliveryQueueMonitor);
//		return isQueueOpen;
//	}
//
//	int GetAbntCount(void)
//	{
//		MutexGuard lock(deliveryQueueMonitor);
//		return AbntsStatus.GetCount();
//	}
//
//	int GetQueueSize(void)
//	{
//		MutexGuard lock(deliveryQueueMonitor);
//		return deliveryQueue.size();
//	}
//
//	void Schedule(const AbntAddr& abnt, bool onBusy=false)
//	{
//		string strAbnt = abnt.toString();
//		smsc_log_debug(logger, "Schedule %s", strAbnt.c_str());
//		MutexGuard lock(deliveryQueueMonitor);
//		if(AbntsStatus.Exists(strAbnt.c_str()))
//		{
//			smsc_log_info(logger, "Abonent %s already scheduled. total = %d (%d)", strAbnt.c_str(), total, deliveryQueue.size());
//			return;
//		}
//		time_t schedTime = time(0);
//		if(onBusy)
//		{
//			schedTime += schedTimeOnBusy;
//			smsc_log_info(logger, "Abonent %s was BUSY waiting up to %s", strAbnt.c_str(), ctime(&schedTime));
//		}
//		deliveryQueue.insert(multimap<time_t, AbntAddr>::value_type(schedTime, abnt));
//		
//		SchedParam schedParam = {Idle, 1};
//		AbntsStatus.Insert(strAbnt.c_str(), schedParam);
////		AbntsStatus.Insert(strAbnt.c_str(), Idle);
//		deliveryQueueMonitor.notify();
//		total++;
//		smsc_log_info(logger, "Add %s. total = %d (%d) (sched rows = %d) (on time %s)", strAbnt.c_str(), total, deliveryQueue.size(), schedParam.count, ctime(&schedTime));
//	}
//
//	void Reschedule(const AbntAddr& abnt, int resp_status = smsc::system::Status::OK) // bool toHead = false
//	{
//		bool toHead = false;
//		string strAbnt = abnt.toString();
//		smsc_log_debug(logger, "Reschedule %s", strAbnt.c_str());
//		MutexGuard lock(deliveryQueueMonitor);
//		if(!AbntsStatus.Exists(strAbnt.c_str()))
//		{
//			smsc_log_debug(logger, "Rescheduling %s canceled (abonent is not in hash).", strAbnt.c_str());
//			return;
//		}
//		
//		if(resp_status == smsc::system::Status::OK) toHead = true;
//
////		uint8_t *status = AbntsStatus.GetPtr(strAbnt.c_str());
//		SchedParam *schedParam = AbntsStatus.GetPtr(strAbnt.c_str());		
////		if(*status == AlertHandled)
//		if(schedParam->abntStatus == AlertHandled)
//		{
//			smsc_log_debug(logger, "ALERT_NOTIFICATION for %s has accepted previously.", strAbnt.c_str());
//			toHead = true;
//		}
//		
//		schedParam->abntStatus = Idle;
////		*status = Idle;
//
//		if(toHead)
//		{
//			deliveryQueue.insert(multimap<time_t, AbntAddr>::value_type(time(0), abnt));
//			schedParam->count++;
//			deliveryQueueMonitor.notify();
//			smsc_log_info(logger, "Rescheduling %s to Head (sched rows = %d)", strAbnt.c_str(), schedParam->count);
//		}
//		else 
//		{
//			if(schedParam->count == 0)
//			{
//				time_t t = CalcTimeDelivery(resp_status);
//				deliveryQueue.insert(multimap<time_t, AbntAddr>::value_type(t, abnt));
//				schedParam->count++;
//				smsc_log_info(logger, "Rescheduling %s to Tail on %s (sched rows = %d)", strAbnt.c_str(), ctime(&t), schedParam->count);
//			}
//			else
//				smsc_log_info(logger, "Rescheduling %s canceled - event already exists (sched rows = %d)", strAbnt.c_str(), schedParam->count);
//		}
//
//	}
//
//	void RegisterAlert(const AbntAddr& abnt)
//	{
//		string strAbnt = abnt.toString();
//		smsc_log_debug(logger, "RegisterAlert for %s", strAbnt.c_str());
//		MutexGuard lock(deliveryQueueMonitor);
//		if(!AbntsStatus.Exists(strAbnt.c_str()))
//		{
//			smsc_log_debug(logger, "Registration alert for %s canceled (abonent is not in hash).", strAbnt.c_str());
//			return;
//		}
//
////		uint8_t *status = AbntsStatus.GetPtr(strAbnt.c_str());
//		SchedParam *schedParam = AbntsStatus.GetPtr(strAbnt.c_str());		
//
////		if(*status == Idle)
//		if(schedParam->abntStatus == Idle)
//		{
//			deliveryQueue.insert(multimap<time_t, AbntAddr>::value_type(time(0), abnt));
//			schedParam->count++;
//			smsc_log_info(logger, "Registering Alert and rescheduling %s to Head (sched rows = %d)", strAbnt.c_str(), schedParam->count);
//			deliveryQueueMonitor.notify();
//		}
//		else
//		{
////			*status = AlertHandled;
//			schedParam->abntStatus = AlertHandled;
//			smsc_log_info(logger, "Registering Alert for %s (sched rows = %d)", strAbnt.c_str(), schedParam->count);
//		}
//	}
//
//	bool Get(AbntAddr& abnt)
//	{
//		MutexGuard lock(deliveryQueueMonitor);
//		int pause = GetDeliveryTime()-time(0);
//
//		smsc_log_debug(logger, "pause = %d", pause);
//		if(pause > 0)
//		{
//			if(0 == deliveryQueueMonitor.wait(pause*1000))
//                smsc_log_debug(logger, "recieved a notify.");
//			else
//				smsc_log_debug(logger, "timeout has passed.");
//		}
//
//		if(!isQueueOpen)
//		{	
//			smsc_log_info(logger, "Queue was closed.");
//			return false;
//		}
//		
//		if(!deliveryQueue.empty())
//		{
//			multimap<time_t, AbntAddr>::iterator It;
//			time_t	t;
//
//			It = deliveryQueue.begin();
//			t = It->first;
//
//			if(t > time(0))
//			{
//				smsc_log_debug(logger, "Delivery time is not reached yet.");
//				return false;
//			}
//			abnt = It->second;
//			deliveryQueue.erase(It);
//			string strAbnt = abnt.toString();
//			if(AbntsStatus.Exists(strAbnt.c_str()))
//			{
//				SchedParam *schedParam = AbntsStatus.GetPtr(strAbnt.c_str());
//				if(schedParam->count > 0) schedParam->count--;
////				uint8_t *status = AbntsStatus.GetPtr(strAbnt.c_str());
////				if(*status == Idle)
//				if(schedParam->abntStatus == Idle)
//				{	
//					//*status = InProcess;
//					schedParam->abntStatus = InProcess;
//					smsc_log_info(logger, "Abonent %s ready to delivery (sched rows = %d).", strAbnt.c_str(), schedParam->count);
//					return true;
//				}
//				else
//				{
//					smsc_log_info(logger, "Abonent %s already in delivery (sched rows = %d).", strAbnt.c_str(), schedParam->count);
//					return false;
//				}
//				return true;
//			}
//			else
//			{
//				smsc_log_debug(logger, "Abonent %s is not exists in the hash.", strAbnt.c_str());
//			    return false;
//			}
//		}
//		
//		smsc_log_debug(logger, "deliveryQueue is empty.");
//		return false;
//	}
//
//	void Remove(const AbntAddr& abnt)
//	{
//		string strAbnt = abnt.toString();
//		smsc_log_debug(logger, "Remove %s",strAbnt.c_str());
//		MutexGuard lock(deliveryQueueMonitor);
//		if(AbntsStatus.Exists(strAbnt.c_str()))
//		{
//			AbntsStatus.Delete(strAbnt.c_str());
//			total--;
//			smsc_log_info(logger, "Remove %s total = %d (%d, %d)", strAbnt.c_str(), total, deliveryQueue.size(), AbntsStatus.GetCount());
//		}
//		else
//			smsc_log_debug(logger, "Remove %s canceled (abonent is not in hash).", strAbnt.c_str());
//	}
//	void Erase(void)
//	{
//		smsc_log_debug(logger, "Erase");
//		MutexGuard lock(deliveryQueueMonitor);
//		smsc_log_info(logger, "Queue size = %d, Hash size = %d, total = %d", deliveryQueue.size(), AbntsStatus.GetCount(), total);
//		deliveryQueue.erase(deliveryQueue.begin(), deliveryQueue.end());
//		AbntsStatus.Empty();
//		total = 0;
//		smsc_log_info(logger, "Erased. (%d %d %d)", deliveryQueue.size(), AbntsStatus.GetCount(), total);
//	}
//
//private:
//
//	time_t CalcTimeDelivery(int err = -1)
//	{
//		if(err == -1) return time(0) + dt;
//		map<int, time_t>::iterator It;
//		It = scheduleTable.find(err);
//		if(It == scheduleTable.end())
//			return time(0) + dt;
//		return time(0) + It->second;
//	}
//
//	time_t GetDeliveryTime(void)
//	{
//		multimap<time_t, AbntAddr>::iterator It;
//		time_t t = time(0) + default_wait;
//		if(!deliveryQueue.empty())
//		{
//			It = deliveryQueue.begin();
//			t = It->first;
//		}
//		return t;
//	}
//};


//class DeliveryQueue
//{
//	list<SchedItem*>	deliveryQueue;
//	EventMonitor		deliveryQueueMonitor;
//	Hash<abnt_stat_t>	AbntsStatus;
//		
//	time_t				dt;
//	uint32_t			total;
//	bool				isQueueOpen;
//
//	smsc::logger::Logger *logger;
//
//public:
//
//	DeliveryQueue(time_t _dt = 5): dt(_dt), total(0), isQueueOpen(false), logger(smsc::logger::Logger::getInstance("smsc.mcisme")){}
//	DeliveryQueue(const DeliveryQueue& addr){}
//	virtual ~DeliveryQueue(){Erase();}
//
//	void OpenQueue(void)
//	{
//		MutexGuard lock(deliveryQueueMonitor);
//		isQueueOpen = true;
//		deliveryQueueMonitor.notify();
//	}
//
//	void CloseQueue(void)
//	{
//		MutexGuard lock(deliveryQueueMonitor);
//		isQueueOpen = false;
//		deliveryQueueMonitor.notify();
//	}
//
//	bool isQueueOpened(void)
//	{
//		MutexGuard lock(deliveryQueueMonitor);
//		return isQueueOpen;
//	}
//
//	void Schedule(const AbntAddr& abnt)
//	{
//		string strAbnt = abnt.toString();
//		smsc_log_debug(logger, "DeliveryQueue: Schedule %s", strAbnt.c_str());
//		MutexGuard lock(deliveryQueueMonitor);
//		if(AbntsStatus.Exists(strAbnt.c_str()))
//		{
//			smsc_log_debug(logger, "DeliveryQueue: Abonent %s already scheduled. total = %d (%d)", strAbnt.c_str(), total, deliveryQueue.size());
//			return;
//		}
//		SchedItem* item = new SchedItem(time(0), abnt);
//		deliveryQueue.push_front(item);
//		AbntsStatus.Insert(strAbnt.c_str(), Idle);
//		deliveryQueueMonitor.notify();
//		total++;
//		smsc_log_debug(logger, "DeliveryQueue: Add %s. total = %d (%d)", strAbnt.c_str(), total, deliveryQueue.size());
//	}
//
//	void Reschedule(const AbntAddr& abnt, bool toHead = false)
//	{
//		string strAbnt = abnt.toString();
//		smsc_log_debug(logger, "DeliveryQueue: Reschedule %s", strAbnt.c_str());
//		MutexGuard lock(deliveryQueueMonitor);
//		if(!AbntsStatus.Exists(strAbnt.c_str()))
//		{
//			smsc_log_debug(logger, "DeliveryQueue: Rescheduling %s canceled (abonent is not in hash).", strAbnt.c_str());
//			return;
//		}
//		
//		uint8_t *status = AbntsStatus.GetPtr(strAbnt.c_str());
//		if(*status == AlertHandled)
//		{
//			smsc_log_debug(logger, "DeliveryQueue: ALERT_NOTIFICATION for %s has accepted previously.", strAbnt.c_str());
//			toHead = true;
//		}
//		
//		*status = Idle;
//
//		if(toHead)
//		{
//			SchedItem* item = new SchedItem(time(0), abnt);
//			deliveryQueue.push_front(item);
//			deliveryQueueMonitor.notify();
//			smsc_log_debug(logger, "DeliveryQueue: Rescheduling %s to Head", strAbnt.c_str());
//		}
//		else
//		{
//			time_t t = CalcTimeDelivery();
//			SchedItem* item = new SchedItem(t, abnt);
//			deliveryQueue.push_back(item);
//			deliveryQueue.sort();
//			smsc_log_debug(logger, "DeliveryQueue: Rescheduling %s to Tail on %s", strAbnt.c_str(), ctime(&t));
//		}
//	}
//
//	void RegisterAlert(const AbntAddr& abnt)
//	{
//		string strAbnt = abnt.toString();
//		smsc_log_debug(logger, "DeliveryQueue: RegisterAlert for %s", strAbnt.c_str());
//		MutexGuard lock(deliveryQueueMonitor);
//		if(!AbntsStatus.Exists(strAbnt.c_str()))
//		{
//			smsc_log_debug(logger, "DeliveryQueue: Registration alert for %s canceled (abonent is not in hash).", strAbnt.c_str());
//			return;
//		}
//
//		uint8_t *status = AbntsStatus.GetPtr(strAbnt.c_str());
//
//		if(*status == Idle)
//		{
//			SchedItem* item = new SchedItem(time(0), abnt);
//			deliveryQueue.push_front(item);
//			deliveryQueueMonitor.notify();
//			smsc_log_debug(logger, "DeliveryQueue: Registering Alert and rescheduling %s to Head", strAbnt.c_str());
//		}
//		else
//		{
//			*status = AlertHandled;
//			smsc_log_debug(logger, "DeliveryQueue: Registering Alert for %s", strAbnt.c_str());
//		}
//	}
//
//	bool Get(AbntAddr& abnt)
//	{
//		smsc_log_debug(logger, "DeliveryQueue: Get");
//		MutexGuard lock(deliveryQueueMonitor);
//		int pause = GetDeliveryTime()-time(0);
//
//		smsc_log_debug(logger, "DeliveryQueue: pause = %d", pause);
//		if(pause > 0)
//		{
//			if(0 == deliveryQueueMonitor.wait(pause*1000))
//                smsc_log_debug(logger, "DeliveryQueue: recieved a notify.");
//			else
//				smsc_log_debug(logger, "DeliveryQueue: timeout has passed.");
//		}
//
//		if(!isQueueOpen)
//		{	
//			smsc_log_debug(logger, "DeliveryQueue: Queue was closed.");
//			return false;
//		}
//
//		if(!deliveryQueue.empty())
//		{
//			SchedItem* item = deliveryQueue.front();
//			if(item->time > time(0))
//			{
//				smsc_log_debug(logger, "DeliveryQueue: Delivery time is not reached yet.");
//				return false;
//			}
//
//			deliveryQueue.pop_front();
//			abnt = item->abnt;
//			string strAbnt = abnt.toString();
//			delete item;
//			if(AbntsStatus.Exists(strAbnt.c_str()))
//			{
//				uint8_t *status = AbntsStatus.GetPtr(strAbnt.c_str());
//				if(*status == Idle)
//				{	
//					*status = InProcess;
//					smsc_log_debug(logger, "DeliveryQueue: Abonent %s ready to delivery.", strAbnt.c_str());
//					return true;
//				}
//				else
//				{
//					smsc_log_debug(logger, "DeliveryQueue: Abonent %s already in delivery.", strAbnt.c_str());
//					return false;
//				}
//				return true;
//			}
//			else
//			{
//				smsc_log_debug(logger, "DeliveryQueue: Abonent %s is not exists in the hash.", strAbnt.c_str());
//			    return false;
//			}
//		}
//		
//		smsc_log_debug(logger, "DeliveryQueue: deliveryQueue is empty.");
//		return false;
//	}
//
//	void Remove(const AbntAddr& abnt)
//	{
//		string strAbnt = abnt.toString();
//		smsc_log_debug(logger, "DeliveryQueue: Remove %s",strAbnt.c_str());
//		MutexGuard lock(deliveryQueueMonitor);
//		if(AbntsStatus.Exists(strAbnt.c_str()))
//		{
//			AbntsStatus.Delete(strAbnt.c_str());
//			total--;
//			smsc_log_debug(logger, "DeliveryQueue:Remove %s total = %d (%d, %d)", strAbnt.c_str(), total, deliveryQueue.size(), AbntsStatus.GetCount());
//		}
//		else
//			smsc_log_debug(logger, "DeliveryQueue: Remove %s canceled (abonent is not in hash).", strAbnt.c_str());
//	}
//	void Erase(void)
//	{
//		smsc_log_debug(logger, "DeliveryQueue: Erase");
//		MutexGuard lock(deliveryQueueMonitor);
//		list<SchedItem*>::iterator	begin = deliveryQueue.begin();
//		list<SchedItem*>::iterator	end = deliveryQueue.begin();
//		list<SchedItem*>::iterator	i;
//		smsc_log_debug(logger, "DeliveryQueue: Queue size = %d, Hash size = %d, total = %d", deliveryQueue.size(), AbntsStatus.GetCount(), total);
//		AbntsStatus.Empty();
//		total = 0;
//		for(i = begin; i != end; i++)
//		{
//			SchedItem* item = deliveryQueue.front();
//			deliveryQueue.pop_front();
//			delete item;
//		}
//		smsc_log_debug(logger, "DeliveryQueue: Errased. (%d %d %d)", deliveryQueue.size(), AbntsStatus.GetCount(), total);
//	}
//
//private:
//
//	time_t CalcTimeDelivery(void)
//	{
//		return time(0) + dt;
//	}
//
//	time_t GetDeliveryTime(void)
//	{
//		time_t t = time(0) + default_wait;
//		if(!deliveryQueue.empty())
//		{
//			SchedItem* item = deliveryQueue.front();
//			t = item->time;
//		}
//		return t;
//	}
//};


//class DeliveryQueue
//{
//	multimap<time_t, AbntAddr>	deliveryQueue;
//	EventMonitor				deliveryQueueMonitor;
//	Hash<abnt_status_t>			AbntsStatus;
//	Mutex						AbntsStatusMutex;
//	
//	time_t		dt;
//	uint32_t	total;	
//public:
//
//	DeliveryQueue(time_t _dt = 5): dt(_dt), total(0){}
//	DeliveryQueue(const DeliveryQueue& addr){}
//	virtual ~DeliveryQueue(){Erase();}
//
//	void Schedule(const AbntAddr& abnt)
//	{
//		MutexGuard lock(deliveryQueueMonitor);
//		if(AbntsStatus.Exists(abnt.toString().c_str())) return;
//		deliveryQueue.insert(multimap<time_t, AbntAddr>::value_type(CalcTimeDelivery(), abnt));
//		AbntsStatus.Insert(abnt.toString().c_str(), 1);
////		deliveryQueueMonitor.notify();
//		total++;
//		printf("DeliveryQueue: Add %s total = %d (%d)\n", abnt.toString().c_str(), total, deliveryQueue.size());
//	}
//
//	void Reschedule(const AbntAddr& abnt, bool toHead = false)
//	{
//		MutexGuard lock(deliveryQueueMonitor);
//		if(toHead)
//			deliveryQueue.insert(multimap<time_t, AbntAddr>::value_type(time(0)-1, abnt));
//		else
//			deliveryQueue.insert(multimap<time_t, AbntAddr>::value_type(CalcTimeDelivery(), abnt));
////		deliveryQueueMonitor.notify();
////		printf("DeliveryQueue: Add%s\n", abnt.toString().c_str());
//	}
//
//	bool Get(AbntAddr& abnt)
//	{
//		MutexGuard lock(deliveryQueueMonitor);
//		multimap<time_t, AbntAddr>::iterator It;
//
//		if(!deliveryQueue.empty())
//		{
////		    if(0 != deliveryQueueMonitor.wait(timeout))
////			return false;
//		
//			It = deliveryQueue.begin();
//			abnt = It->second;
//			deliveryQueue.erase(It);
////			deliveryQueueMonitor.notify();
//			if(AbntsStatus.Exists(abnt.toString().c_str()))
//			    return true;
//			else
//			    return false;
//		}
//		return false;
//
//	}
//
//	void Remove(const AbntAddr& abnt)
//	{
//		MutexGuard lock(deliveryQueueMonitor);
//		if(AbntsStatus.Exists(abnt.toString().c_str()))
//			AbntsStatus.Delete(abnt.toString().c_str());
//		total--;
//		printf("DeliveryQueue:Remove %s total = %d (%d)\n", abnt.toString().c_str(), total, deliveryQueue.size());
//
//	}
//
//	time_t GetDeliveryTime(void)
//	{
////		printf("DeliveryQueue::GetDeliveryTime\n");
//		MutexGuard lock(deliveryQueueMonitor);
//		multimap<time_t, AbntAddr>::iterator It;
//		time_t t = time(0) + 3;
//		if(!deliveryQueue.empty())
//		{
//			It = deliveryQueue.begin();
//			t = It->first;
//		}
//		return t;
//	}
//
//	void Erase(void)
//	{
//		AbntsStatus.Empty();
//		deliveryQueue.erase(deliveryQueue.begin(), deliveryQueue.end());
//	}
//
//private:
//	
//	//void AddToQueue(const AbntAddr& abnt, time_t delivery_time)
//	//{
//	//	MutexGuard lock(deliveryQueueMutex);
//	//	deliveryQueue.insert(map<time_t, AbntAddr>::value_type(delivery_time, abnt));
//	//}
//	//
//	//void AddToHash(const AbntAddr& abnt, abnt_status_t status)
//	//{
//	//	MutexGuard lock(AbntsStatusMutex);
//	//	AbntsStatus.Insert(abnt.toString().c_str(), status);
//	//}
//
//	time_t CalcTimeDelivery(void)
//	{
//		return time(0) + dt;
//	}
//
//};
