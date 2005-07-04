#include "TrafficRecord.h"

namespace scag{
namespace stat{

TrafficRecord::TrafficRecord(long min_, long hour_, long day_, long month_) :
	min(min_), hour(hour_), day(day_), month(month_), date(date_)
{
}

TrafficRecord::~TrafficRecord()
{
}

void TrafficRecord::inc(const tm& tmDate)
{
	bool update = false;
	if(year_ != tmDate.tm_year){
		min = 1;
		hour = 1;
		day = 1;
		month = 1;
		update = true;
	}else if(month_ != tmDate.tm_month){
		min = 1;
		hour = 1;
		day = 1;
		month = 1;
		update = true;
	}else if(day_ != tmDate.tm_mday){
		min = 1;
		hour = 1;
		day = 1;
		month++;
		update = true;
	}else if(hour_ != tmDate.tm_hour){
		hour = 1;
		min = 1;
		day++;
		month++;
		update = true;
	}else if(min_ != tmDate.tm_min){
		min = 1;
		hour++;
		day++;
		month++;
		update = true;
	}else{
		min++;
		hour++;
		day++;
		month++;
	}
	
	if(update){
		year_ = tmDate.tm_year;
		month_ = tmDate.tm_month;
		day_ = tmDate.tm_day;
		hour_ = tmDate.tm_hour;
		min_ = tmDate.tm_min;
	}
	
}

}
}
