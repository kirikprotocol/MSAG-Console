#ifndef TrafficRecord_dot_h
#define TrafficRecord_dot_h

#include <time.h>

namespace scag {
namespace stat {

class TrafficRecord
{
public:
	TrafficRecord(long mincnt, long hourcnt, long daycnt, long monthcnt, uint8_t year_, uint8_t month_, uint8_t day_, uint8_t hour_, uint8_t min_);
	virtual ~TrafficRecord();
	void inc(const tm& tmDate);
    void getRouteData(long& mincnt_, long& hourcnt_, long& daycnt_, long& monthcnt_, uint8_t& year_, uint8_t& month_, uint8_t& day_, uint8_t& hour_, uint8_t& min_);
protected:
	long mincnt;
	long hourcnt;
	long daycnt;
	long monthcnt;
	
	// Update date
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t min;
}

}
}

#endif
