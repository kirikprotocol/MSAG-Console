//------------------------------------
//  FSStorage.cpp
//  Routman Michael, 2005-2006
//------------------------------------

#include "Storage.hpp"
#include "FSStorage.hpp"
#include "string.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>

int errno;


#include "vector"
#include "list"


#include <core/buffers/File.hpp>
#include <core/synchronization/Mutex.hpp>

#include <mcisme/AbntAddr.hpp>
#include <mcisme/DeliveryQueue.hpp>

#include <stdio.h>

namespace smsc { namespace mcisme 
{

using std::vector;
using std::list;
using core::buffers;
using sms::Address;
using core::synchronization::Mutex;
using core::synchronization::MutexGuard;
using smsc::util::config::ConfigView;

static int parseTime(const char* str)
{
    int hour, minute, second;
    if (!str || str[0] == '\0' ||
        sscanf(str, "%02d:%02d:%02d", 
                    &hour, &minute, &second) != 3) return -1;
    
    return hour*3600+minute*60+second;
}

FSStorage::FSStorage(): logger(smsc::logger::Logger::getInstance("mci.fsStor"))
{
	zero_idx_cell = new uint8_t[sizeof(idx_file_cell)];
	zero_dat_cell = new uint8_t[sizeof(dat_file_cell)];
//	zero_event_cell = new uint8_t[sizeof(event_cell)];

	memset((void*)zero_idx_cell, 0x00, sizeof(idx_file_cell));
	memset((void*)zero_dat_cell, 0x00, sizeof(dat_file_cell));
//	memset((void*)event_cell, 0x00, sizeof(event_cell));	
}

FSStorage::~FSStorage()
{
	delete[] zero_dat_cell;
	delete[] zero_idx_cell;
//	delete[] zero_event_cell;
	CloseFiles();
	smsc_log_debug(logger, "FSStorage: Destroed.\n");
}

int FSStorage::Init(ConfigView* storageConfig, DeliveryQueue* pDeliveryQueue)
{
	MutexGuard	lock(mut);

	smsc_log_debug(logger, "Reading Storage parameters.");
	string location;
	try { location = storageConfig->getString("location"); } catch (...){ location = "./";
		smsc_log_warn(logger, "Parameter <MCISme.Storage.DBFilesPath> missed. Default value is './'.");}
	
	try { maxEvents = storageConfig->getInt("maxEvents"); } catch (...){maxEvents = 20;
		smsc_log_warn(logger, "Parameter <MCISme.Storage.MaxEvents> missed. Default value is 20.");}

	string sEventLifeTime;
	try { sEventLifeTime = storageConfig->getString("eventLifeTime"); } catch (...){sEventLifeTime = "24:00:00";
		smsc_log_warn(logger, "Parameter <MCISme.Storage.EventLifeTime> missed. Default value is '24:00:00'.");}
	eventLifeTime = parseTime(sEventLifeTime.c_str());

	string sPolicy;
	try { sPolicy = storageConfig->getString("eventRegistrationPolicy"); } catch (...){sPolicy = "RejectNewEvent";
		smsc_log_warn(logger, "Parameter <MCISme.Storage.eventRegistrationPolicy> missed. Default value is 'RejectNewEvent'.");}

	try { bdFilesIncr = storageConfig->getInt("bdFilesIncr"); } catch (...){bdFilesIncr = 1000;
		smsc_log_warn(logger, "Parameter <MCISme.Storage.bdFilesIncr> missed. Default value is 1000.");}

	pathDatFile = location + "dat_file";
	pathIdxFile = location + "idx_file";
	pathTransFile = location + "trans_file";
	if(sPolicy == "RejectNewEvent")	policy = REJECT_NEW_EVENT;
	if(maxEvents > MAX_EVENTS) maxEvents = MAX_EVENTS;
	if(bdFilesIncr > MAX_BDFILES_INCR) bdFilesIncr = MAX_BDFILES_INCR;

	//printf("pathDatFile = %s\n", pathDatFile.c_str());
	//printf("pathIdxFile = %s\n", pathIdxFile.c_str());
	//printf("pathTransFile = %s\n", pathTransFile.c_str());
	//printf("Policy = %d\n", policy);
	//printf("maxEvents = %d\n", maxEvents);
	//printf("bdFilesIncr = %d\n", bdFilesIncr);
	//exit(0);
//	printf("size of idx = %d, size of dat = %d\n", sizeof(idx_file_cell), sizeof(dat_file_cell));
//	printf("FSStorage: Init storage.\n");
	if(0 != OpenFiles())
		return 1;

	smsc_log_debug(logger, "FSStorage: All files opened");
	if(0 != CompleteTransaction())
		return 2;

	if(0 != LoadEvents(pDeliveryQueue))
		return 3;

	return 0;	
}

int FSStorage::Init(const string& location, time_t _eventLifeTime, uint8_t _maxEvents, DeliveryQueue* pDeliveryQueue)
{
	MutexGuard	lock(mut);

	pathDatFile = location + "dat_file";
	pathIdxFile = location + "idx_file";
	pathTransFile = location + "trans_file";
	eventLifeTime = _eventLifeTime;
	maxEvents <= MAX_EVENTS? maxEvents = _maxEvents: maxEvents = MAX_EVENTS;
	
//	printf("size of idx = %d, size of dat = %d\n", sizeof(idx_file_cell), sizeof(dat_file_cell));
//	printf("FSStorage: Init storage.\n");
	if(0 != OpenFiles())
		return 1;

	smsc_log_debug(logger, "FSStorage: All files opened");
	if(0 != CompleteTransaction())
		return 2;

	if(0 != LoadEvents(pDeliveryQueue))
		return 3;
	
}

void FSStorage::addEvent(const AbntAddr& CalledNum, const MCEvent& event, time_t schedTime)
{
	MutexGuard	lock(mut);

	if(hashAbnt.Exists(CalledNum))
	{
		dat_file_cell	AbntEvents;

		LoadAbntEvents(CalledNum, &AbntEvents);
		KillExpiredEvents(&AbntEvents);
		AddAbntEvent(&AbntEvents, event);
		SaveAbntEvents(CalledNum, &AbntEvents);
	}
	else
		CreateAbntEvents(CalledNum, event, schedTime);
}

void FSStorage::setSchedTime(const AbntAddr& CalledNum, time_t schedTime)
{
	MutexGuard	lock(mut);

	if(hashAbnt.Exists(CalledNum))
	{
		dat_file_cell	AbntEvents;

		LoadAbntEvents(CalledNum, &AbntEvents);
		AbntEvents.schedTime = schedTime;
		SaveAbntEvents(CalledNum, &AbntEvents);
	}
}

bool FSStorage::getEvents(const AbntAddr& CalledNum, vector<MCEvent>& events)
{
	MutexGuard	lock(mut);

	MCEvent				e;

	if(hashAbnt.Exists(CalledNum))
	{
		dat_file_cell	AbntEvents;

		LoadAbntEvents(CalledNum, &AbntEvents);
		if(KillExpiredEvents(&AbntEvents))
			if(AbntEvents.event_count == 0)
			{
				DestroyAbntEvents(CalledNum);
//				smsc_log_debug(logger, "FSStorage: getEvents: All Events are expired.\n");
				return 0;
			}
			else
				SaveAbntEvents(CalledNum, &AbntEvents);

		for(int i = 0; i < AbntEvents.event_count; i++)
		{
			e.id = AbntEvents.events[i].id;
			e.dt = AbntEvents.events[i].date;
			memcpy((void*)&(e.caller), (void*)&(AbntEvents.events[i].calling_num), sizeof(e.caller));
			events.push_back(e);
		}					
		return 1;
	}
	else
		return 0;
}

void FSStorage::deleteEvents(const AbntAddr& CalledNum, const vector<MCEvent>& events)
{
	MutexGuard	lock(mut);

	if(hashAbnt.Exists(CalledNum))
	{
		dat_file_cell	AbntEvents;

		LoadAbntEvents(CalledNum, &AbntEvents);
		RemoveAbntEvents(&AbntEvents, events);
		KillExpiredEvents(&AbntEvents);
		if(AbntEvents.event_count == 0)
		{
			DestroyAbntEvents(CalledNum);
			return;
		}
		SaveAbntEvents(CalledNum, &AbntEvents);
//		smsc_log_debug(logger, "FSStorage: deleteEvents: End deleting Events of Existing Abonent\n");
	}
	else
	{
//		smsc_log_debug(logger, "FSStorage: deleteEvents: Attempt DELETING Events of non-existence Abonent\n");
		return;
	}
}

int	FSStorage::OpenFiles(void)
{
	try
	{
		if(File::Exists(pathDatFile.c_str()))
			dat_file.RWOpen(pathDatFile.c_str());
		else
			dat_file.RWCreate(pathDatFile.c_str());

		dat_file.SetUnbuffered();
	}
	catch(FileException ex)
	{
		smsc_log_debug(logger, "FSStorage: error dat_file - %s\n", ex.what());
		return 1;
	}
	try
	{
		if(File::Exists(pathIdxFile.c_str()))
			idx_file.RWOpen(pathIdxFile.c_str());
		else
			idx_file.RWCreate(pathIdxFile.c_str());

		idx_file.SetUnbuffered();
	}
	catch(FileException ex)
	{
		smsc_log_debug(logger, "FSStorage: error idx_file - %s\n", ex.what());
		dat_file.Close();
		return 2;
	}

	try
	{
		if(File::Exists(pathTransFile.c_str()))
			trans_file.RWOpen(pathTransFile.c_str());
		else
			trans_file.RWCreate(pathTransFile.c_str());

		trans_file.SetUnbuffered();
	}
	catch(FileException ex)
	{
		smsc_log_debug(logger, "FSStorage: error trans_file - %s\n", ex.what());
		dat_file.Close();
		idx_file.Close();
		return 3;
	}
	return 0;
}

void FSStorage::CloseFiles(void)
{
	dat_file.Close();
	idx_file.Close();
	trans_file.Close();
}

int FSStorage::LoadEvents(DeliveryQueue* pDeliveryQueue)
{
	uint32_t	buf_size = sizeof(idx_file_cell);//*1000;
	uint8_t*	buf = new uint8_t[buf_size];

//	char buf1[]="1234567890";
//	char buf2[1000];
	
	if(!buf)
	{
//		smsc_log_debug(logger, "FSStorage: Error in LoadEvents - new[] faild\n");
		return 1;
	}

	smsc_log_info(logger, "FSStorage: Loading DataBase from files.......\n");
	
	time_t	t = time(0);
	uint32_t	cell_num=0;
	
	try
	{
		uint32_t		rb, i, count; //, cell_num=0;
		off_t			size;
		dat_file_cell	AbntEvents;

		idx_file.SeekEnd(0);
		if(0 == (size = idx_file.Pos()))
		{
			smsc_log_debug(logger, "FSStorage: FSStorage: Nothing to load - idx_file is empty.\n");
			delete[] buf;
			return 0;
		}
		idx_file.Seek(0);
		
		while(rb = idx_file.Read((void*)buf, buf_size))
		{
//			if( (size - idx_file.Pos())/sizeof(idx_file_cell) < 1000)
			count = rb / sizeof(idx_file_cell);
			if( (count * sizeof(idx_file_cell)) != rb)
			{
				delete[] buf; 
				return 1;
			}
			for(i = 0; i < count; i++)
			{
				if(0 == memcmp((void*)&buf[i*sizeof(idx_file_cell)], (void*)zero_idx_cell, sizeof(idx_file_cell)))
				{
					freeCells.AddCell(cell_num);
				}
				else
				{
					AbntAddr abnt(&buf[i*sizeof(idx_file_cell)]);
					LoadAbntEvents(abnt, &AbntEvents);
					hashAbnt.Insert(abnt, cell_num);
					pDeliveryQueue->Schedule(abnt, false, AbntEvents.schedTime);
				}
				cell_num++;
			}
		}
	}
	catch(FileException ex)
	{
//		smsc_log_debug(logger, "FSStorage: Error in LoadEvents - %s. Was read = %d\n", ex.what(), cell_num);
		delete[] buf;
		return 0;
	}

	delete[] buf;
	return 0;
}


int FSStorage::CompleteTransaction(void)
{
	return 0;
}
int FSStorage::CreateAbntEvents(const AbntAddr& CalledNum, const MCEvent& event, time_t schedTime)
{
	cell_t			cell;
	idx_file_cell	idx;
	dat_file_cell	dat;

	memset((void*)&dat, 0xFF, sizeof(dat));
	dat.event_count = 1;
	dat.events[0].date = time(0);
	dat.events[0].id = 0;
	dat.schedTime = schedTime;
	memcpy((void*)&dat.inaccessible_num, (void*)CalledNum.getAddrSig(), sizeof(dat.inaccessible_num));
	memcpy((void*)&(dat.events[0].calling_num), (void*)&(event.caller), sizeof(dat.events[0].calling_num));
	memcpy((void*)&idx.inaccessible_num, (void*)CalledNum.getAddrSig(), sizeof(idx.inaccessible_num));

	if(freeCells.Empty())
		IncrStorage(bdFilesIncr);

	cell = freeCells.GetCell();
	try
	{
		idx_file.Seek(cell*sizeof(idx_file_cell));
		idx_file.Write((void*)&idx, sizeof(idx_file_cell));

		dat_file.Seek(cell*sizeof(dat_file_cell), SEEK_SET);
		dat_file.Write((void*)&dat, sizeof(dat_file_cell));
	}
	catch(FileException ex)
	{
		smsc_log_debug(logger, "FSStorage: Error in CreateAbntEvents - %s\n", ex.what());
		return 1;
	}
	hashAbnt.Insert(CalledNum, cell);
	return 0;
}

int FSStorage::DestroyAbntEvents(const AbntAddr& CalledNum)
{
	cell_t		cell = hashAbnt.Get(CalledNum);	

	try
	{
		dat_file.Seek(cell*sizeof(dat_file_cell), SEEK_SET);
		dat_file.Write((void*)zero_dat_cell, sizeof(dat_file_cell));
		idx_file.Seek(cell*sizeof(idx_file_cell), SEEK_SET);
		idx_file.Write((void*)zero_idx_cell, sizeof(idx_file_cell));
	}
	catch(FileException ex)
	{
		smsc_log_debug(logger, "FSStorage: DestroyAbntEvents: !! Error !! - %s\n", ex.what());
		return 1;
	}

	hashAbnt.Remove(CalledNum);
	freeCells.AddCell(cell);

	return 0;
}
int FSStorage::SaveAbntEvents(const AbntAddr& CalledNum, const dat_file_cell* pAbntEvents)
{
//	Добавить транзакции.

	cell_t		cell = hashAbnt.Get(CalledNum);	

	try
	{
		dat_file.Seek(cell*sizeof(dat_file_cell), SEEK_SET);
		dat_file.Write((void*)pAbntEvents, sizeof(dat_file_cell));
	}
	catch(FileException ex)
	{
		smsc_log_debug(logger, "FSStorage: Error in SaveAbntEvents - %s\n", ex.what());
		return 1;
	}
	return 0;
}

int FSStorage::LoadAbntEvents(const AbntAddr& CalledNum, dat_file_cell* pAbntEvents)
{
	cell_t	cell = 	hashAbnt.Get(CalledNum);

	try
	{
		dat_file.Seek(cell*sizeof(dat_file_cell), SEEK_SET);
		dat_file.Read((void*)pAbntEvents, sizeof(dat_file_cell));
	}
	catch(FileException ex)
	{
		smsc_log_debug(logger, "FSStorage: Error in LoadAbntEvents - %s\n", ex.what());
		return 1;
	}
	return 0;
}

int FSStorage::AddAbntEvent(dat_file_cell* pAbntEvents, const MCEvent& event)
{
//	if(policy == REJECT_NEW_EVENT)

	if(pAbntEvents->event_count >= maxEvents)
		return 1;

	pAbntEvents->events[pAbntEvents->event_count].date = time(0);
	pAbntEvents->events[pAbntEvents->event_count].id = pAbntEvents->events[pAbntEvents->event_count-1].id + 1;
	memcpy((void*)&(pAbntEvents->events[pAbntEvents->event_count].calling_num), (void*)&(event.caller.full_addr), sizeof(pAbntEvents->events[pAbntEvents->event_count].calling_num));
	pAbntEvents->event_count++;
	return 0;
}

int FSStorage::RemoveAbntEvents(dat_file_cell* pAbntEvents, const vector<MCEvent>& events)
{
	if(pAbntEvents->event_count == 0)
		return 0;

	for(int i = events.size(); i > 0; i--)
		RemoveEvent(pAbntEvents, events[i-1]);
	
	return pAbntEvents->event_count;
}

int FSStorage::RemoveEvent(dat_file_cell* pAbntEvents, const MCEvent& event)
{
	int	event_num;
	for(event_num = 0; event_num < pAbntEvents->event_count; event_num++)
		if(pAbntEvents->events[event_num].id == event.id)
			break;

	if(event_num == pAbntEvents->event_count)return 1;
	else if(event_num < pAbntEvents->event_count-1)
		memcpy((void*)&(pAbntEvents->events[event_num]), (void*)&(pAbntEvents->events[event_num+1]), (pAbntEvents->event_count-event_num-1)*sizeof(event_cell));
	memset((void*)&(pAbntEvents->events[pAbntEvents->event_count-1]), 0xFF, sizeof(event_cell));
	pAbntEvents->event_count--;
//	smsc_log_debug(logger, "FSStorage: RemoveEvent killed: %d\n", killed);
	return 0;
}



int FSStorage::KillExpiredEvents(dat_file_cell* pAbntEvents)
{
	MCEvent	e;
	int		killed = 0;

	for(int i = 0; i < pAbntEvents->event_count; i++)
	{
		time_t dt = pAbntEvents->events[i].date;
		smsc_log_debug(logger, "FSStorage: KillExpiredEvents: (Event.date(='%s') + eventLifeTime(=%ld sec.) )=%ld", 
		               ctime(&dt), eventLifeTime, dt + eventLifeTime);
		if((dt + eventLifeTime) < time(0))
		{
			e.id = pAbntEvents->events[i].id;		// так как удаление идет по id, то остальное не заполняю.
			if(0 == RemoveEvent(pAbntEvents, e))
				killed++;
		}
	}
	smsc_log_debug(logger, "FSStorage:KillExpiredEvents: killed: %d\n", killed);
	return killed;
}

int FSStorage::IncrStorage(const uint32_t& num_cells)
{
	uint32_t last_num, size;
	try
	{
		idx_file.SeekEnd(0);
		last_num = ((uint32_t)idx_file.Pos()) / sizeof(idx_file_cell);
		size = (uint32_t)idx_file.Size();
	}
	catch(FileException ex)
	{
		smsc_log_debug(logger, "FSStorage: Error in IncrSorage - %s (Size return %d)\n", ex.what(), size);
		return 3;
	}
	if(0 != IncrIdxFile(num_cells))
		return 1;
	if(0 != IncrDatFile(num_cells))
		return 2;
	for(uint32_t i=0; i<num_cells; i++) freeCells.AddCell(last_num+i);
	return 0;
}

int FSStorage::IncrIdxFile(const uint32_t& num_cells)
{
	size_t		buf_size = num_cells * sizeof(idx_file_cell);
	uint8_t*	buf = new uint8_t[buf_size];
	
	if(!buf)
	{
		smsc_log_debug(logger, "FSStorage: Error in IncrIdxFile - new[] faild\n");
		return 1;
	}
	memset((void*)buf, 0x00, buf_size);
	try
	{
		idx_file.SeekEnd(0);		
		idx_file.Write((void*)buf, buf_size);
	}
	catch(FileException ex)
	{
		delete[] buf;
		smsc_log_debug(logger, "FSStorage: Error in IncrIdxFile - %s\n", ex.what());
		return 2;
	}
	delete[] buf;
	return 0;
}

int FSStorage::IncrDatFile(const uint32_t& num_cells)
{
	uint32_t	buf_size = sizeof(dat_file_cell)*num_cells;
	uint8_t* buf = new uint8_t[buf_size];

	if(!buf)
	{
		smsc_log_debug(logger, "FSStorage: Error in IncrDatFile - new[] faild\n");
		return 1;
	}
	memset((void*)buf, 0x00, buf_size);
	try
	{
		dat_file.SeekEnd(0);		
		dat_file.Write((void*)buf, buf_size);
	}
	catch(FileException ex)
	{
		delete[] buf;
		smsc_log_debug(logger, "FSStorage: Error in IncrDatFile - %s\n", ex.what());
		return 2;
	}
	delete[] buf;
	return 0;
}

};	//  namespace msisme
};	//  namespace smsc


























//void FSStorage::addEvent(AbntAddr CalledNum, MCEvent event)
//{
//	dat_file_cell	dat;
//	idx_file_cell	idx[100000];
////	event_cell		cell;
//
//	memset((void*)&dat, 0x00, sizeof(dat));
//	memset((void*)&idx, 0x00, sizeof(idx));
//
////	write(fd_dat, (void*)&dat, sizeof(dat));
//	write(fd_idx, (void*)&idx, sizeof(idx));
////	printf("addEvent %d %d %d\n", sizeof(dat), sizeof(time_t), sizeof(idx));
//}


	//if(-1 == (fd_dat = open("dat_file", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)))
	//{
	//	perror("Error fd_dat");
	//	return 1;
	//}
	//if(-1 == (fd_idx = open("idx_file", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)))
	//{
	//	perror("Error fd_dat");
	//	close(fd_dat);
	//	return 2;
	//}
	//if(-1 == (fd_trans = open("trans_file", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)))
	//{
	//	perror("Error fd_dat");
	//	close(fd_dat);
	//	close(fd_idx);
	//	return 3;
	//}

		//int fd;
	//int c;

	//if(-1 == (fd = open("test", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)))
	//{
	//	perror("Error fd_dat");
	//	return 1;
	//}
	//write(fd, buf1, sizeof(buf1));
	//lseek(fd, 0, SEEK_SET);
	//c = read(fd, buf2, 1000);

	//printf("%d %s\n", c, buf2);


	//int fd = open("idx_file", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	//if(fd==-1)
	//	printf("!!!!!!!!!!!!!!!!\n");
 //   
	//struct ::stat st;
 //   fstat(fd,&st);
 //   printf("size = %d", (int)st.st_size);
	//printf("End Test !!!!!!!!!!!\n");
	//exit(0);
