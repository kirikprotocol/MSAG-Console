#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <stdio.h>

#include "util/TonNpiAddress.hpp"

namespace smsc {
namespace util {

const MobileAlphabet _MAPAlphabet;

/* ************************************************************************** *
 * IMSIString implementation
 * ************************************************************************** */
MobileAlphabet::MobileAlphabet() : _Numeric("[0-9]"), _Tele("[0-9*#abc]")
  //NOTE: actually it's a subset of GSM-7bit alphabet(3GPP TS 23.038) with Greece chars excluded
  , _Gsm7("[]0-9*#a-zA-Z _.,:;&+=()/'\"?!<>|^~\\{}[%-]")
{ }

//Validates and ouputs no more then 'max_chars' characters according to specified alphaBet.
//NOTE: 'out_buf' MUST have enough capacity to store 'max_chars' + 1 symbols.
bool MobileAlphabet::validateChars(const char * alpha_bet, const unsigned max_chars,
                                   const char * in_buf, char * out_buf)
{
  char  trash[4];
  char  fmt[_MAX_ALPHABET_LEN + sizeof("%NNN%2s ")];

  //compose format string for sscanf()
  snprintf(fmt, sizeof(fmt)-1, "%%%u%s%%2s", max_chars, alpha_bet);
  //parse input string
  int scanned = sscanf(in_buf, fmt, out_buf, trash);
  return (scanned == 1);
}


/* ************************************************************************** *
 * IMSIString implementation
 * ************************************************************************** */

const char * IMSIString::_numberingFmt = "%15[0-9]%2s";

bool IMSIString::fromText(const char * in_text, char * out_str)
{
  char  trash[4];
  int   scanned = sscanf(in_text, _numberingFmt, out_str, trash);
  return (scanned == 1) && (strlen(out_str) >= 5);
}


/* ************************************************************************** *
 * TonNpiAddress implementation
 * ************************************************************************** */

bool TonNpiAddress::fromText(const char * text_str)
{
  if (!text_str || !*text_str)
    return false;

  unsigned      numChars = (unsigned)strlen(text_str);
  int           offset = 0;
  const char *  alphaBet = NULL;

  // determine required alphabet depending on ToN
  if (text_str[0] == '.') { //".ton.npi.signals"
    unsigned  iplan = 0, itype = 0;
    int       scanned = sscanf(text_str, ".%1u.%2u.%n", &itype, &iplan, &offset);

    if ((scanned != 2) || (itype > tonReservedExt) || (iplan > npiReservedExt))
      return false;

    typeOfNumber = (uint8_t)(itype & 0x07);
    numPlanInd = (uint8_t)(iplan & 0x0F);

    if ((itype != TonNpiAddress::tonUnknown)
        && (itype != TonNpiAddress::tonReservedExt)) {
      alphaBet = (itype == TonNpiAddress::tonAlphanum) ? _MAPAlphabet._Gsm7 : _MAPAlphabet._Tele;
    } //else autodetect alphabet
  } else if (text_str[0] == '+') { //isdn international address
    offset = 1;
    typeOfNumber = TonNpiAddress::tonInternational;
    numPlanInd = TonNpiAddress::npiISDNTele_e164;
    alphaBet = _MAPAlphabet._Tele;
  }

  if ((numChars -= offset) > TonNpiAddress::_maxSIGNALS) {
    clear();
    return false; //to many signals
  }

  if (!alphaBet) { //autodetect alphabet
    if (MobileAlphabet::validateChars(_MAPAlphabet._Tele, numChars, text_str + offset, signals)) {
      alphaBet = _MAPAlphabet._Tele;
      typeOfNumber = TonNpiAddress::tonUnknown;
      numPlanInd = TonNpiAddress::npiISDNTele_e164;
    } else if (MobileAlphabet::validateChars(_MAPAlphabet._Gsm7, numChars, text_str + offset, signals)) {
      alphaBet = _MAPAlphabet._Gsm7;
      typeOfNumber = TonNpiAddress::tonAlphanum;
      numPlanInd = TonNpiAddress::npiUnknown;
    }
  } else if (!MobileAlphabet::validateChars(alphaBet, numChars, text_str + offset, signals))
    alphaBet = NULL;

  if (!alphaBet) {
    clear();
    return false;
  }
  length = (uint8_t)strlen((const char*)signals);
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

