#include "core/network/Multiplexer.hpp"
#include "core/buffers/Array.hpp"
#include <stdio.h>

using namespace smsc::core::network;

int main()
{
  Socket s;
  if(s.InitServer("smsc",9876,0)==-1)return -1;
  if(s.StartServer()==-1)return -2;
  Multiplexer m;
  m.add(&s);
  Multiplexer::SockArray ready,error;
  char buf[512];
  for(;;)
  {
    if(m.canRead(ready,error)>0)
    {
      for(int i=0;i<ready.Count();i++)
      {
        if(ready[i]==&s)
        {
          m.add(s.Accept());
        }else
        {
          ready[i]->Read(buf,512);
          ready[i]->Write(buf,512);
          printf(".");fflush(stdout);
        }
      }
    }
  }
}
