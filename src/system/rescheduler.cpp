#include "system/rescheduler.hpp"
#include "util/debug.h"
#include <ctype.h>
#include <memory>

namespace smsc{
namespace system{


RescheduleCalculator::TimeArray RescheduleCalculator::RescheduleTable;
IntHash<RescheduleCalculator::TimeArray*> RescheduleCalculator::CodesTable;
int RescheduleCalculator::DefaultAttemptsLimit;
IntHash<int> RescheduleCalculator::AttemptsLimits;

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

static time_t strtotime(const char*& str)
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

void RescheduleCalculator::ParseTimeLine(const char* timestring,RescheduleCalculator::TimeArray& arr,int& limit)
{
  limit=0;
  Array<const char*> parts;
  Split(timestring,parts);
  for(int i=0;i<parts.Count();i++)
  {
    if(limit==-1)
    {
      __warning2__("records found after infinite element, last element will be used as infinite:%s",timestring);
    }
    const char* p=parts[i];
    time_t t=strtotime(p);
    arr.Push(t);
    while(isspace(*p))p++;
    if(*p==',' || *p==0)
    {
      limit++;
      continue;
    }
    if(*p!=':')throw Exception("Invalid timeline format here:%s",p);
    p++;
    if(*p=='*')
    {
      limit=-1;p++;
    }else
    if(isdigit(*p))
    {
      int n,v;
      sscanf(p,"%d%n",&v,&n);
      if(limit!=-1)limit+=v;
      for(int j=0;j<v-1;j++)arr.Push(t);
      p+=n;
    }else
    {
      throw Exception("Invalid timeline format here:%s",p);
    }
    if(*p!=',' && *p!=0)Exception("Invalid timeline format here:%s",p);
  }
}

void RescheduleCalculator::InitDefault(const char* timestring)throw(Exception)
{
  ParseTimeLine(timestring,RescheduleTable,DefaultAttemptsLimit);
}

void RescheduleCalculator::AddToTable(const char* timeline,const char* codes)
{
  std::auto_ptr<TimeArray> ta(new TimeArray);
  int attcnt;
  ParseTimeLine(timeline,*ta.get(),attcnt);
  Array<const char*> parts;
  Split(codes,parts);
  TimeArray* pta=ta.release();
  for(int i=0;i<parts.Count();i++)
  {
    int code=atoi(parts[i]);
    CodesTable.Insert(code,pta);
    AttemptsLimits.Insert(code,attcnt);
  }
}


time_t RescheduleCalculator::calcNextTryTime(time_t lasttry,int code,int attempt)
{
  if(CodesTable.Exist(code))
  {
    TimeArray *ta=CodesTable.Get(code);
    if(AttemptsLimits.Get(code)!=-1 && attempt>=AttemptsLimits.Get(code))return (time_t)-1;
    if(ta->Count()==0)
    {
      __warning__("Rescheduling table for error code %d is empty!!!");
      return lasttry+60;
    }
    if(attempt>=ta->Count())attempt=ta->Count()-1;
    return lasttry+(*ta)[attempt];
  }
  if(DefaultAttemptsLimit!=-1 && attempt>=DefaultAttemptsLimit)return -1;
  if(RescheduleTable.Count()==0)
  {
    __warning__("Rescheduling table is empty!");
    return lasttry+60;
  }
  if(attempt<0)attempt=0;
  if(attempt>=RescheduleTable.Count())attempt=RescheduleTable.Count()-1;
  return lasttry+RescheduleTable[attempt];
}



}//system
}//smsc
