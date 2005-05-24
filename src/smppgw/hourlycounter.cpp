#include "hourlycounter.h"
#include "logger/Logger.h"

namespace smsc {
namespace smppgw {

HourlyCounter::HourlyCounter() :
	routId_(""),
	date_(time(0)),
	offset_(0)
{
	for(int i=0; i<=719; i++)
		acceptedCnt[i] = 0;
}

HourlyCounter::HourlyCounter(const struct TrafficWriter::routData& rd)
{
	routId_ = rd.routId;
	
	uint16_t netYear = 0;
	memcpy((void*)&netYear, (const void*)rd.date, 2);
	uint16_t year = ntohs(netYear);
	int month = rd.date[2];
	int day = rd.date[3];
	int hh = rd.date[4];
	
	tm rtm;
	rtm.tm_year = year - 1900;
	rtm.tm_mon = month - 1;
	rtm.tm_mday = day;
	rtm.tm_hour = hh;
	rtm.tm_min = 0;
	rtm.tm_sec = 0;
	rtm.tm_isdst = -1;
	
	date_ = mktime(&rtm);
	
	int32_t tmp;
	memcpy((void*)&tmp, (const void*)&rd.offset_, 4);
	offset_ = ntohl(tmp);
	
	for(int i=0; i <= 719; i++){
		int32_t tmp;
		memcpy((void*)&tmp, (const void*)(rd.acceptedCnt + 4*i), 4);
		acceptedCnt[i] = ntohl(tmp);
	}
	
	// Convert to current time
	time_t now = time(0);
    tm tmnow; localtime_r(&now, &tmnow);
	int delay = (int)(std::difftime(now, date_)/3600.);
    if(delay > 0){
        tmnow.tm_min = 0;
        tmnow.tm_sec = 0;
        tmnow.tm_isdst = -1;
        date_ = mktime(&tmnow);
    }
	
	// circl buffer	
	int hour = offset_/2921; hour = (offset_ - hour*2921 - 41)/4;	
	offset_ -= 4*hour;
	if(hour + delay > 719){

		if(delay > 719){
			if(hour == 719)			
				hour = 0;
			else
				++hour;
		}else{
			hour = hour + delay - 720;
		}
	}else{
		hour = hour + delay;
	}
	offset_ += 4*hour;
	
	if(delay - 1 > hour){
		for(int i=0; i<=hour; i++)
			acceptedCnt[i] = 0;
	}else{
		if(delay > 0){
			for(int i=hour-delay+1; i<=hour; i++)
				acceptedCnt[i] = 0;
		}
	}
	
	if(delay - 1 > hour){
		if(720 < delay - 1){
			if(hour < 719){
				for(int i=hour+1; i<=719; i++ )
					acceptedCnt[i] = 0;
			}
		}else{
			if(hour < 719){
				int start = delay == 721 ? hour + 1 : 721 - delay + hour;
				for(int i = start; i<=719; i++)
					acceptedCnt[i] = 0;
			}	
		}
	}		
}

HourlyCounter::HourlyCounter(const struct TrafficWriter::routData& rd, bool update)
{
	routId_ = rd.routId;
	
	uint16_t netYear = 0;
	memcpy((void*)&netYear, (const void*)rd.date, 2);
	uint16_t year = ntohs(netYear);
	int month = rd.date[2];
	int day = rd.date[3];
	int hh = rd.date[4];
	
	tm rtm;
	rtm.tm_year = year - 1900;
	rtm.tm_mon = month - 1;
	rtm.tm_mday = day;
	rtm.tm_hour = hh;
	rtm.tm_min = 0;
	rtm.tm_sec = 0;
	rtm.tm_isdst = -1;
	
	date_ = mktime(&rtm);
	
	int32_t tmp;
	memcpy((void*)&tmp, (const void*)&rd.offset_, 4);
	offset_ = ntohl(tmp);
	
	for(int i=0; i <= 719; i++){
		int32_t tmp;
		memcpy((void*)&tmp, (const void*)(rd.acceptedCnt + 4*i), 4);
		acceptedCnt[i] = ntohl(tmp);
	}

    if(!update)
        return;
	
	// Convert to current time
	time_t now = time(0);
    tm tmnow; localtime_r(&now, &tmnow);
	int delay = (int)(std::difftime(now, date_)/3600.);
    if(delay > 0){
        tmnow.tm_min = 0;
        tmnow.tm_sec = 0;
        tmnow.tm_isdst = -1;
        date_ = mktime(&tmnow);
    }
	
	// circl buffer	
	int hour = offset_/2921; hour = (offset_ - hour*2921 - 41)/4;	
	offset_ -= 4*hour;
	if(hour + delay > 719){

		if(delay > 719){
			if(hour == 719)			
				hour = 0;
			else
				++hour;
		}else{
			hour = hour + delay - 720;
		}
	}else{
		hour = hour + delay;
	}
	offset_ += 4*hour;
	
	if(delay - 1 > hour){
		for(int i=0; i<=hour; i++)
			acceptedCnt[i] = 0;
	}else{
		if(delay > 0){
			for(int i=hour-delay+1; i<=hour; i++)
				acceptedCnt[i] = 0;
		}
	}
	
	if(delay - 1 > hour){
		if(720 < delay - 1){
			if(hour < 719){
				for(int i=hour+1; i<=719; i++ )
					acceptedCnt[i] = 0;
			}
		}else{
			if(hour < 719){
				int start = delay == 721 ? hour + 1 : 721 - delay + hour;
				for(int i = start; i<=719; i++)
					acceptedCnt[i] = 0;
			}	
		}
	}		
}

void HourlyCounter::getRoutData(struct TrafficWriter::routData& rd) const{
	
    strcpy(rd.routId, routId_.c_str());
	
	tm rtm; localtime_r(&date_, &rtm);
	uint16_t year = rtm.tm_year + 1900;
	uint16_t netYear = htons(year);
	memcpy((void*)rd.date, (const void*)&netYear, 2);
	rd.date[2] = rtm.tm_mon + 1;
	rd.date[3] = rtm.tm_mday;
	rd.date[4] = rtm.tm_hour;
	
	int32_t tmp = htonl(offset_);
	memcpy((void*)&rd.offset_, (const void*)&tmp, 4);
	
	for(int i=0; i<=719; i++){
		int32_t tmp = htonl(acceptedCnt[i]);
		memcpy((void*)&rd.acceptedCnt[4*i], (const void*)&tmp, 4);
	}
}

void HourlyCounter::update(int& offset, int& delay, int& value, time_t& dt)
{
    // Convert to current time
	time_t now = time(0);
    tm tmnow; localtime_r(&now, &tmnow);
	delay = (int)(std::difftime(now, date_)/3600.);
    if(delay > 0){
        tmnow.tm_min = 0;
        tmnow.tm_sec = 0;
        tmnow.tm_isdst = -1;
        date_ = mktime(&tmnow);
    }
    dt = date_;
	
	// circl buffer
	char tmplog[500];
	smsc::logger::Logger *log=smsc::logger::Logger::getInstance("hc");	
	int hour = offset_/2921; hour = (offset_ - hour*2921 - 41)/4;	
	offset_ -= 4*hour;		
	if(hour + delay > 719){
			
		if(delay > 719){
			if(hour == 719)			
				hour = 0;
			else
				++hour;
		}else{
			hour = hour + delay - 720;
		}
	}else{
		hour = hour + delay;
	}
	offset_ += 4*hour;
    offset = offset_;
	
	if(delay - 1 > hour){
        for(int i=0; i<=hour; i++)
            acceptedCnt[i] = 0;
        value = ++acceptedCnt[hour];
	}else{
		if(delay > 0){
			for(int i=hour-delay+1; i<=hour; i++)
				acceptedCnt[i] = 0;
		}
        value = ++acceptedCnt[hour];
	}
	
	if(delay - 1 > hour){
		if(720 < delay - 1){
			if(hour < 719){
				for(int i=hour+1; i<=719; i++ )
					acceptedCnt[i] = 0;
			}
		}else{
			if(hour < 719){
				int start = delay == 721 ? hour + 1 : 721 - delay + hour;
				for(int i = start; i<=719; i++)
					acceptedCnt[i] = 0;
			}	
		}
	}
}

int HourlyCounter::HourCount()
{
        int hour = (offset_ - (int)(offset_/2921)*2921 - 41)/4;
        return acceptedCnt[hour];
}

int HourlyCounter::WeekCount(){
        int hour = (offset_ - (int)(offset_/2921)*2921 - 41)/4;
        int count = acceptedCnt[hour];
        for(int i=0; i<=7*24 - 2; i++){
            hour = prev(hour);
            count += acceptedCnt[hour];
        }
        return count;
}

int HourlyCounter::DayCount(){
        int hour = (offset_ - (int)(offset_/2921)*2921 - 41)/4;
        int count = acceptedCnt[hour];
        for(int i=0; i<=22; i++){
            hour = prev(hour);
            count += acceptedCnt[hour];
        }
        return count;
}

int HourlyCounter::MonthCount(){
    int count = 0;
    for(int i=0; i<=719; i++)
        count += acceptedCnt[i];

    return count;
}

int HourlyCounter::prev(int pos){
    if(pos == 0)
        return 719;
    else
        return pos - 1;
}

void HourlyCounter::dump(){
	char tmplog[500];
	smsc::logger::Logger *log=smsc::logger::Logger::getInstance("hc.dump");
	
	sprintf(tmplog, "roudId: %s, offset: %d, cnt0: %d, cnt1: %d, cnt2: %d, cnt3: %d, cnt4: %d, cnt717: %d, cnt718: %d, cnt719: %d", routId_.c_str(), offset_, acceptedCnt[0], acceptedCnt[1], acceptedCnt[2], acceptedCnt[3], acceptedCnt[4], acceptedCnt[717], acceptedCnt[718], acceptedCnt[719]);
	smsc_log_info(log, tmplog );
	
	tm mytm; localtime_r(&date_, &mytm);
	int y = mytm.tm_year + 1900;
	int m = mytm.tm_mon + 1;
	int d = mytm.tm_mday;
	int h = mytm.tm_hour;
	sprintf(tmplog, "y: %d, m: %d, d: %d, h: %d", y, m, d, h);
	smsc_log_info(log, tmplog );
}

HourlyCounter::~HourlyCounter(){
}

}
}
