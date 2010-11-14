#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <stdio.h>

#include "util/TonNpiAddress.hpp"

namespace smsc {
namespace util {

const char * IMSIString::_numberingFmt = "%15[0-9]%2s";

bool IMSIString::fromText(const char * in_text, char * out_str)
{
  char  trash[4];
  int   scanned = sscanf(in_text, _numberingFmt, out_str, trash);
  return (scanned == 1) && (strlen(out_str) >= 5);
}


bool TonNpiAddress::fromText(const char* text)
{
  if (!text || !*text)
      return false;

  char    buff[CAPConst::MAX_SMS_AddressValueLength + 1];
  char *  addr_value = buff;
  int     iplan = 0, itype = 0;
  int     max_scan = 1, scanned = 0;

  memset(buff, 0, sizeof(buff));
  switch (text[0]) {
  case '.': {  //ton.npi.adr
    scanned = sscanf(text, ".%d.%d.%20[0-9a-zA-Z *#.:]s", &itype, &iplan, addr_value);
    max_scan = 3;
  } break;

  case '+': {  //isdn international adr
    scanned = sscanf(text, "+%20[0123456789]s", addr_value);
    iplan = itype = 1;
  } break;

  default:    //isdn unknown or alpha-numeric adr
    size_t txt_len = strlen(text);
    if (txt_len > 0xFF)
        return false;
    length = (unsigned char)txt_len;

    scanned = sscanf(text, "%20[0123456789]s", addr_value);
    if (scanned == 1 && (length == strlen(addr_value))) {
        iplan = 1; /*itype = 0;*/   // isdn unknown
    } else {
        if (length <= CAPConst::MAX_SMS_AddressValueLength) {
            /*iplan = 0;*/ itype = 5;   //alpha-numeric adr
            addr_value = (char*)text;
            scanned = 1;
        }
    }
  }
  if ((scanned < max_scan) || (iplan > 0xFF) || (itype > 0xFF))
    return false;

  numPlanInd = (unsigned char)iplan;
  typeOfNumber = (unsigned char)itype;
  memcpy(signals, addr_value, length = (unsigned char)strlen((const char*)addr_value));
  signals[length] = 0;
  return true;
}

//use at least TonNpiAddress::_strSZ chars buffer
int TonNpiAddress::toString(char * buf, bool ton_npi/* = true*/,
                            unsigned buflen/* = TonNpiAddress::_strSZ*/) const
{
  int n = 0;
  if (length) {
    if (ton_npi)
      n = snprintf(buf, buflen - 1, ".%u.%u.", (unsigned)typeOfNumber, (unsigned)numPlanInd);
    else if (interISDN())
      buf[n++] = '+';
    n += snprintf(buf + n, buflen - n - 1, "%s", getSignals());
  }
  buf[n] = 0;
  return n;
}

}//util
}//smsc

