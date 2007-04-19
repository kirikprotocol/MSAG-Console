#ifndef __SMSC_INFO_SME_TRAFFICCONTROL_HPP__
# define __SMSC_INFO_SME_TRAFFICCONTROL_HPP__ 1

# include <util/timeslotcounter.hpp>

class TrafficControl
{
    static EventMonitor   trafficMonitor;
    static TimeSlotCounter<int> incoming;
    static TimeSlotCounter<int> outgoing;
    static bool                 stopped;

public:

  static void incOutgoing();
  static void incIncoming();
  static void stopControl();
  static void startControl();
};


#endif
