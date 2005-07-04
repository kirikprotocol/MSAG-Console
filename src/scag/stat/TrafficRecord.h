#ifndef TrafficRecord_dot_h
#define TrafficRecord_dot_h

#include <time.h>

namespace scag {
namespace stat {

class TrafficRecord
{
public:
	TrafficRecord(long min_, long hour_, long day_, long mont_, time_t date_);
	virtual ~TrafficRecord();
	void inc(const tm& tmDate);
protected:
	long min;
	long hour;
	long day;
	long month;
	
	// Update date
	int year_;
	int month_;
	int day_;
	int hour_;
	int min_;
}

}
}

#endif
