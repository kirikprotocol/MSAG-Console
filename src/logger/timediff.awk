#
# $Id$
# This script may be used to process logger output to find out long delays.
# Typical usage:
# cat msag.log | grep 'SOME INTERESTING QUITE REGULAR EVENT' | awk -f THISSCRIPT.awk | less
#
BEGIN { otm = 0; om=0; od=0; omd=0; }
{ d=substr($2,1,2)+0; m=substr($2,4,2)+0; hh=substr($3,1,2)+0; mm=substr($3,4,2)+0; ss=substr($3,7,2)+0; ms=substr($3,10,3)+0;
  if ( om != m || od != d ) {
    if ( om != 0 ) { omd = omd + 86400; }
    om = m; od = d;
  }
  tm = (omd + hh*3600 + mm*60 + ss)*1000 + ms;
  if ( otm != 0 ) {
    printf "%ld %s\n", tm-otm, $0;
  }
  otm = tm;
}
