#include "system/rescheduler.hpp"
#include "util/debug.h"

namespace smsc{
namespace system{


smsc::core::buffers::Array<time_t> RescheduleCalculator::RescheduleTable;

void RescheduleCalculator::Init(const char* timestring)throw(Exception)
{
  time_t result;
  int pos;
  int val;
  char type;
  while(*timestring)
  {
    result=0;
    while(*timestring && *timestring!=',')
    {
      sscanf(timestring,"%d%c%n",&val,&type,&pos);
      __trace2__("RescheduleCalculator: scanned %d%c",val,type);
      switch(type)
      {
        case 'D':
        case 'd':result+=val*60*60*24;break;
        case 'H':
        case 'h':result+=val*60*60;break;
        case 'M':
        case 'm':result+=val*60;break;
        case 'S':
        case 's':result+=val;break;
        default: Exception("Invalid time format");
      }
      timestring+=pos;
    }
    __trace2__("RescheduleCalculator: scanned time = %d",result);
    RescheduleTable.Push(result);
    if(*timestring==',')timestring++;
  }
}

time_t RescheduleCalculator::calcNextTryTime(time_t lasttry,int attempt)
{
  if(attempt>=RescheduleTable.Count())attempt=RescheduleTable.Count()-1;
  return lasttry+RescheduleTable[attempt];
}



};//system
};//smsc
