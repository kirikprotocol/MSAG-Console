#ifndef __SMSC_INFO_SME_TRAFFICCONTROL_HPP__
#define __SMSC_INFO_SME_TRAFFICCONTROL_HPP__ 1

#include <util/timeslotcounter.hpp>
#include <core/synchronization/EventMonitor.hpp>

using smsc::core::synchronization::EventMonitor;

class TrafficControl {

public:
  static void incOutgoing();
  static void incIncoming();
  static void stopControl();
  static void startControl();

private:
  static EventMonitor   trafficMonitor;
  static TimeSlotCounter<int> incoming;
  static TimeSlotCounter<int> outgoing;
  static bool                 stopped;
};

#endif

