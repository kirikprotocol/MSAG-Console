#include "debug.h"

int main(int argc, char** argv)
{
  int kk=0;
  __trace2__ ( "kk = " );
  __trace2__ ( "kk = %d", kk );
  __watch__(kk);
  __watchdog__( kk != 0);
  __ret0_if_fail__( kk != 0 );
}
