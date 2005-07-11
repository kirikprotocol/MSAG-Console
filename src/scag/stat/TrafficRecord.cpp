#include "TrafficRecord.h"

namespace scag{
namespace stat{

TrafficRecord::TrafficRecord(long mincnt_, long hourcnt_, long daycnt_, long monthcnt_, uint8_t year_, uint8_t month_, uint8_t day_, uint8_t hour_, uint8_t min_) :
	mincnt(mincnt_), hourcnt(hourcnt_), daycnt(daycnt_), monthcnt(monthcnt_), 
    year(year_), month(month_), day(day_), hour(hour_), min(min_)
{
}

TrafficRecord::~TrafficRecord()
{
}

void TrafficRecord::inc(const tm& tmDate)
{
	bool update = false;
	if(year != tmDate.tm_year){
		mincnt = 1;
		hourcnt = 1;
		daycnt = 1;
		monthcnt = 1;
		update = true;
	}else if(month != tmDate.tm_month){
		mincnt = 1;
		hourcnt = 1;
		daycnt = 1;
		monthcnt = 1;
		update = true;
	}else if(day != tmDate.tm_mday){
		mincnt = 1;
		hourcnt = 1;
		daycnt = 1;
		monthcnt++;
		update = true;
	}else if(hour != tmDate.tm_hour){
		hourcnt = 1;
		mincnt = 1;
		daycnt++;
		monthcnt++;
		update = true;
	}else if(min != tmDate.tm_min){
		mincnt = 1;
		hourcnt++;
		daycnt++;
		monthcnt++;
		update = true;
	}else{
		mincnt++;
		hourcnt++;
		daycnt++;
		monthcnt++;
	}
	
	if(update){
		year = tmDate.tm_year;
		month = tmDate.tm_month;
		day = tmDate.tm_day;
		hour = tmDate.tm_hour;
		min = tmDate.tm_min;
	}
	
}

void TrafficRecord::reset(const tm& tmDate)
{
	bool update = false;
	if(year != tmDate.tm_year){
		mincnt = 0;
		hourcnt = 0;
		daycnt = 0;
		monthcnt = 0;
		update = true;
	}else if(month != tmDate.tm_month){
		mincnt = 0;
		hourcnt = 0;
		daycnt = 0;
		monthcnt = 0;
		update = true;
	}else if(day != tmDate.tm_mday){
		mincnt = 0;
		hourcnt = 0;
		daycnt = 0;
		monthcnt++;
		update = true;
	}else if(hour != tmDate.tm_hour){
		hourcnt = 0;
		mincnt = 0;
		daycnt++;
		monthcnt++;
		update = true;
	}else if(min != tmDate.tm_min){
		mincnt = 0;
		hourcnt++;
		daycnt++;
		monthcnt++;
		update = true;
	}else{
		mincnt++;
		hourcnt++;
		daycnt++;
		monthcnt++;
	}
	
	if(update){
		year = tmDate.tm_year;
		month = tmDate.tm_month;
		day = tmDate.tm_day;
		hour = tmDate.tm_hour;
		min = tmDate.tm_min;
	}
	
}

void TrafficRecord::getRouteData(long& mincnt_, long& hourcnt_, long& daycnt_, long& monthcnt_, uint8_t& year_, uint8_t& month_, uint8_t& day_, uint8_t& hour_, uint8_t& min_)
{
    mincnt_ = mincnt;
    hourcnt_ = hourcnt;
    daycnt_ = daycnt;
    monthcnt_ = monthcnt;
    year_ = year;
    month_ = month;
    day_ = day;
    hour_ = hour;
    min_ = min;
}

}
}
