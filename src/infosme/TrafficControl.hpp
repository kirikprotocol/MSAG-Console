#ifndef __SMSC_INFO_SME_TRAFFICCONTROL_HPP__
#define __SMSC_INFO_SME_TRAFFICCONTROL_HPP__ 1

#include <util/timeslotcounter.hpp>
#include <core/synchronization/EventMonitor.hpp>

using smsc::core::synchronization::EventMonitor;

class TrafficControl {

public:
  static void incOutgoing( unsigned nchunks );
  static void incIncoming();
  static void stopControl();
  static void startControl();

private:
  static EventMonitor   trafficMonitor;
  static smsc::util::TimeSlotCounter<int> incoming;
  static smsc::util::TimeSlotCounter<int> outgoing;
  static bool                 stopped;
};

#endif

