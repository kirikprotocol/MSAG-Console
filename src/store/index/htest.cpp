#include <stdio.h>
#include "core/buffers/DiskHash.hpp"
#include "logger/Logger.h"

using namespace smsc::core::buffers;
using namespace smsc::util;

#include "SmsKeys.hpp"

typedef DiskHash<Int64Key,IdLttKey> SmsIdDiskHash;

int main(int argc,char* argv[])
{
  smsc::logger::Logger::Init();
  SmsIdDiskHash h;
  h.Create("test.dat",1000,true);
  for(int i=0;i<1000000;i++)
  {
    if((i%1000)==0)
    {
      printf("%d\n",i);
    }
    h.Insert(i,IdLttKey(i,i));
  }

  return 0;
}
