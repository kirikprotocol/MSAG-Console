#include "system/rescheduler.hpp"
#include "util/debug.h"
#include <ctype.h>
#include <memory>

namespace smsc{
namespace system{


RescheduleCalculator::TimeArray RescheduleCalculator::RescheduleTable;
IntHash<RescheduleCalculator::TimeArray*> RescheduleCalculator::CodesTable;

static void Split(const char* str,Array<const char*>& parts)
{
  while(str && *str)
  {
    while(isspace(*str))str++;
    parts.Push(str);
    str=strchr(str,',');
    if(str)
    {
      str++;
    }
  }
}

static time_t strtotime(const char* str)
{
  int val;
  char type;
  int pos;
  time_t result=0;

  while(sscanf(str,"%d%c%n",&val,&type,&pos)==2)
  {
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
      default: abort();throw Exception("Invalid time format:%c",type);
    }
    str+=pos;
  }
  return result;
}

void RescheduleCalculator::InitDefault(const char* timestring)throw(Exception)
{
  Array<const char*> parts;
  Split(timestring,parts);
  for(int i=0;i<parts.Count();i++)
  {
    RescheduleTable.Push(strtotime(parts[i]));
  }
}

void RescheduleCalculator::AddToTable(const char* timeline,const char* codes)
{
  Array<const char*> parts;
  std::auto_ptr<TimeArray> ta(new TimeArray);
  Split(timeline,parts);
  for(int i=0;i<parts.Count();i++)
  {
    ta.get()->Push(strtotime(parts[i]));
  }
  parts.Clean();
  Split(codes,parts);
  for(int i=0;i<parts.Count();i++)
  {
    CodesTable.Insert(atoi(parts[i]),ta.get());
  }
  ta.release();
}


time_t RescheduleCalculator::calcNextTryTime(time_t lasttry,int code,int attempt)
{
  if(CodesTable.Exist(code))
  {
    TimeArray *ta=CodesTable.Get(code);
    if(ta->Count()==0)
    {
      __warning2__("Rescheduling table for error code %d is empty!!!");
      return 60;
    }
    if(attempt>=ta->Count())attempt=ta->Count()-1;
    return lasttry+(*ta)[attempt];
  }
  if(RescheduleTable.Count()==0)
  {
    __warning__("Rescheduling table is empty!");
    return 60;
  }
  if(attempt<0)attempt=0;
  if(attempt>=RescheduleTable.Count())attempt=RescheduleTable.Count()-1;
  return lasttry+RescheduleTable[attempt];
}



};//system
};//smsc
