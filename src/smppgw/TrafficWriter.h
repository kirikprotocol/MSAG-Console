#ifndef TrafficWriter_dot_h
#define TrafficWriter_dot_h 1

#include <stdio.h>
#include <string>
#include <time.h>
#include "sms/sms_const.h"
#include <netinet/in.h>

namespace smsc {
namespace smppgw {

using namespace smsc::sms;

const int DATE_FIELD_LENGTH = 5;
const int CNT_FIELD_LENGTH = 4;
const int OFFSET_FIELD_LENGTH = 4;

class TrafficWriter
{

public:

struct routData {
	char routId[MAX_ROUTE_ID_TYPE_LENGTH];
	char date[DATE_FIELD_LENGTH];
	char offset_[4];
	char acceptedCnt[2880];
	routData(const char* routId_, time_t dt){
		
		std::strcpy(routId, routId_);
		
		tm tmDate;
		localtime_r(&dt, &tmDate);
		uint16_t year = tmDate.tm_year + 1900;
		char month = tmDate.tm_mon + 1;
		char day = tmDate.tm_mday;
		char hour = tmDate.tm_hour;
		uint16_t netYear = htons(year);
		
		std::memcpy((void*)date, (const void*)&netYear, 2);
		date[2] = month;
		date[3] = day;
		date[4] = hour;
				
		int32_t start_cnt = 1;
		int32_t val = 0;
		val = htonl(start_cnt);
		printf("val: %d\n", val);
		std::memcpy((void*)acceptedCnt, (const void*)&val, 4);
		
		for(char* p=acceptedCnt+4; p<=acceptedCnt+4*719; p = p + 4){
			int32_t reset = 0;
			int32_t val = 0;
			val = htonl(reset);
			std::memcpy((void*)(p), (const void*)&val, 4);
		}
		
		for(int i=0; i<=3; i++)	
			offset_[i] = 0;
	}
	routData(){
		for(int i=0; i<= MAX_ROUTE_ID_TYPE_LENGTH-1; i++)
			routId[i] = 0;
			
		for(int i=0; i<=4; i++)
			date[i] = 0;
			
		for(int i=0; i<=3; i++)
			offset_[i] = 0;
			
		for(int i=0; i<=2879; i++)
			acceptedCnt[i] = 0;
	}
	void offset(int offset){
		int32_t val = htonl(offset);
		std::memcpy((void*)offset_, (const void*)&val, 4);
	}
};

public:
	TrafficWriter(const char* location_);
	TrafficWriter();
	~TrafficWriter();
	void newRec(const char* routId, int& offset, time_t dt);
	void updateRec(int offset, int delay, int value, time_t dt);
	int readAt(int pos, routData* rd);
	void setLocation(const char* loc);
    void updateRec(int offset, const struct routData rd);
private:
    bool createTrafficDir();
    bool createDir(const std::string& dir);
	FILE* cfPtr;
	std::string filename;
	std::string location;
	void timeToCharPtr(time_t dat, char* date);
};

}
}

#endif
