#ifndef HourlyCounter_dot_h
#define HourlyCounter_dot_h 1

#include <string>
#include "TrafficWriter.h"

namespace smsc {
namespace smppgw {

class HourlyCounter {
public:
    int HourCount();
    int DayCount();
    int WeekCount();
    int MonthCount();
	void update(int& offset, int& delay, int& value, time_t& dt);
	void dump();
	void getRoutData(struct TrafficWriter::routData& rd) const;
	HourlyCounter();
	HourlyCounter(const struct TrafficWriter::routData& rd);
    HourlyCounter(const struct TrafficWriter::routData& rd, bool update);
	virtual ~HourlyCounter();
protected:
    int prev(int pos);
	std::string routId_;
	time_t date_;
	int32_t offset_;
	int32_t acceptedCnt[720];
};

}
}

#endif
