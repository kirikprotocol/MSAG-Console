#include <stdio.h>
#include "core/buffers/FixedRecordFile.hpp"
#include <vector>

using namespace smsc::core::buffers;
using namespace std;

struct Record{
  uint32_t x;
  uint32_t y;
  void Read(File& f)
  {
    x=f.ReadNetInt32();
    y=f.ReadNetInt32();
  }
  void Write(File& f)const
  {
    f.WriteNetInt32(x);
    f.WriteNetInt32(y);
  }
  static uint32_t Size(){return 4+4;}
};

int main(int argc,char* argv[])
{
  FixedRecordFile<Record> frf("TEST",0x0100);
  frf.Open("test.bin");
  File::offset_type off;
  Record rec;
  vector<File::offset_type> tokill;
  while((off=frf.Read(rec)))
  {
    printf("Read:%d,%d\n",rec.x,rec.y);
    if(rand()&0x16)
    {
      tokill.push_back(off);
    }
  }

  for(vector<File::offset_type>::iterator it=tokill.begin();it!=tokill.end();it++)
  {
    frf.Delete(*it);
  }

  for(int i=0;i<100;i++)
  {
    rec.x=i;
    rec.y=i*2;
    frf.Append(rec);
  }
  return 0;
}
