#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "util/templates/Formatters.h"

int main(int argc, char** argv)
{
  try{
    smsc::util::templates::OutputFormatter f("Your accurate balance is \\$$$string import=param1$$");
    smsc::util::templates::ContextEvironment ce;
    ce.exportStr("param1","hello");
    std::string s;
    smsc::util::templates::GetAdapter* ga=0;
    f.format(s,*ga,ce);
  }catch(...)
  {
    printf("Shit happened\n");
  }
}
