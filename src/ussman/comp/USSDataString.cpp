#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <string.h>

#include "inman/common/CBSDefs.hpp"
using smsc::cbs::GSM7_Language;
using smsc::cbs::CBS_DCS;
using smsc::cbs::parseCBS_DCS;

#include "inman/common/cvtutil.hpp"
using smsc::cvtutil::unpack7BitPadded2Text;
using smsc::cvtutil::estimate7BitAsText;

using smsc::cvtutil::packTextAs7BitPadded;
using smsc::cvtutil::estimateTextAs7Bit;

#include "ussman/comp/USSDataString.hpp"

namespace smsc {
namespace ussman {
namespace comp {

static const char _HexChars[] = "0123456789ABCDEF";

/* ************************************************************************** *
 * class USSDataString implementation
 * ************************************************************************** */
//Converts data to ASCIIZ-string representation storing it in provided buffer.
//Note: it's a caller responsibility to provide necessary output buffer space
//      (at maximum _max_StrForm_sz).
//Returns number of chars ought to be typed.
//NOTE: if provided buffer length isn't enough, i.e (return_value >= buf_len),
//      no output is performed.
uint16_t USSDataString::toString(char * out_buf, uint16_t buf_len/* = _max_StrForm_sz*/) const
{
  uint16_t rval = 0;
  *out_buf = 0;

  if (isDataAsLatin1(&rval)) {
    if (rval < buf_len)
      getDataAsLatin1(out_buf);
    return rval;
  }
  
  char * pCur = out_buf;
  if (isDataAsUCS2(&rval)) {
    rval = rval*3 + (uint16_t)sizeof("UCS2: 0x") - 1;
    if (rval >= buf_len)
      return rval;

    strcpy(pCur, "UCS2: 0x");
    pCur += sizeof("UCS2: 0x") - 1;

    for (uint16_t i = 0; i < _data.size(); ++i) {
      *pCur++ = _HexChars[_data[i] >> 4]; 
      *pCur++ = _HexChars[_data[i] & 0x0F]; 
      if (i%2)
        *pCur++ = ' ';
    }
  } else if (isPackedDCS()) {
    strcpy(pCur, "dCS("); pCur += 4;
    *pCur++ = _HexChars[_dCS >> 4];
    *pCur++ = _HexChars[_dCS & 0x0F];
    strcpy(pCur, "h): 0x"); pCur += 6;

    for (uint16_t i = 0; i < _data.size(); ++i) {
      *pCur++ = _HexChars[_data[i] >> 4]; 
      *pCur++ = _HexChars[_data[i] & 0x0F]; 
    }
  }
  *pCur++ = 0;
  return (uint16_t)(pCur - out_buf);
}


//Checks if data is set and if 'p_len' argument is set, estimates resulted
//number of octets in packed/encoded form.
bool USSDataString::isDataAsPackedDCS(uint16_t * p_len/* = NULL*/) const
{
  if (isLatin1Text()) {
    if (p_len)
      *p_len = (uint16_t)estimateTextAs7Bit(getText(), length(), NULL);
    return true;
  }
  if (isPackedDCS() || isUCS2Text()) {
    if (p_len)
      *p_len = _data.size();
    return true;
  }
  return false;
}
//Exports data as packed one according to DCS.
//Returns number of octets exported.
//Note: it's a caller responsibility to provide necessary output buffer
//      space. Usually, it doesn't exceed the _max_USSDStringLength, but 
//      it's safer to determine it by calling isDataAsPackedDCS().
uint16_t USSDataString::getDataAsPackedDCS(uint8_t & out_dcs, uint8_t * out_buf) const
{
  if (isLatin1Text()) {
    out_dcs = _dCS;
    return (uint16_t)packTextAs7BitPadded(getText(), length(), out_buf);
  }
  if (isPackedDCS() || isUCS2Text()) {
    out_dcs = _dCS;
    memcpy(out_buf, _data.get(), _data.size());
    return _data.size();
  }
  return 0;
}

//Checks if data may be represented as Latin1 text, if it's true and 
//'p_len' argument is set, estimates resulted number of Latin1 chars
//(including language prefix).
bool USSDataString::isDataAsLatin1(uint16_t * p_len/* = NULL*/) const
{
  if (isLatin1Text()) {
    if (p_len)
      *p_len = length();
    return true;
  }
  if (isPackedDCS()) {
    CBS_DCS    parsedDCS;
    if (parseCBS_DCS(_dCS, parsedDCS) == CBS_DCS::dcGSM7Bit) {
      if (p_len)
        *p_len = (uint16_t)estimate7BitAsText(_data.get(), (unsigned)_data.size(), NULL);
      return true;
    }
  }
  return false;
}

//Exports data as Latin1 text if possible.
//Returns number of chars exported.
//Note: it's a caller responsibility to provide necessary output buffer
//      space. Usually, it doesn't exceed the _max_USSDLatin1Chars, but 
//      it's safer to determine it by calling isDataAsLatin1().
uint16_t USSDataString::getDataAsLatin1(char * out_str) const
{
  *out_str = 0;
  if (isLatin1Text()) {
    strcpy(out_str, getText());
    return length();
  }
  if (isPackedDCS()) {
    CBS_DCS    parsedDCS;
    if (parseCBS_DCS(_dCS, parsedDCS) == CBS_DCS::dcGSM7Bit) {
      uint16_t resLen = (uint16_t)unpack7BitPadded2Text(_data.get(), (unsigned)_data.size(), (uint8_t *)out_str);
      return resLen;
    }
  }
  return 0;
}

//Exports data as Latin1 text if possible
/*
bool USSDataString::getDataAsLatin1(std::string & out_str) const
{
  if (isLatin1Text()) {
    out_str.assign(getText(), length());
    return true;
  }
  if (isPackedDCS()) {
    CBS_DCS    parsedDCS;
    if (parseCBS_DCS(_dCS, parsedDCS) == CBS_DCS::dcGSM7Bit) {
      unpack7BitPadded2Text(_data.get(), (unsigned)_data.size(), out_str);
      return true;
    }
  }
  return false;
}
*/
//Checks if data may be represented as UCS2 text, if it's true and 
//if 'p_len' argument is set, estimates resulted number of octets
//(including language prefix).
bool USSDataString::isDataAsUCS2(uint16_t * p_len/* = NULL*/) const
{
  if (isUCS2Text()) {
    if (p_len)
      *p_len = _data.size();
    return true;
  }
  if (isPackedDCS()) {
    CBS_DCS    parsedDCS;
    if ((parseCBS_DCS(_dCS, parsedDCS) == CBS_DCS::dcUCS2)
         && !parsedDCS.compressed && !parsedDCS.UDHind ) {
      if (p_len)
        *p_len = _data.size();
      return true;
    }
  }
  return false;
}

//Exports data as UCS2 text if possible.
//Returns number of octets exported.
//Note: it's a caller responsibility to provide necessary output buffer
//      space. Usually, it doesn't exceed the 2*_max_USSDUcs2Chars, but 
//      it's safer to determine it by calling isDataAsUCS2().
uint16_t USSDataString::getDataAsUCS2(uint8_t * out_buf) const
{
  if (isUCS2Text()) {
    memcpy(out_buf, _data.get(), _data.size());
    return _data.size();
  }
  if (isPackedDCS()) {
    CBS_DCS    parsedDCS;
    if ((parseCBS_DCS(_dCS, parsedDCS) == CBS_DCS::dcUCS2)
         && !parsedDCS.compressed && !parsedDCS.UDHind ) {
      uint16_t ofs  = (parsedDCS.lngPrefix == cbs::CBS_DCS::lng4UCS2) ? 2 : 0;
      memcpy(out_buf, _data.get() + ofs, _data.size() - ofs);
      return (_data.size() - ofs);
    }
  }
  return 0;
}

//Verifies that Latin1 data string has a terminating zero
//Note: applicable only in case of ussdLATIN1_TEXT
void USSDataString::setEOS(void)
{
  if (_data.size() && _data.atLast()) {
    //add terminating zero
    _data.resize(_data.size() + 1);
    _data.at(_data.size() - 1) = 0;
  }
}

//checks Latin1 data string for language prefix and sets DCS accordingly.
void USSDataString::setLanguageDCS(void)
{
  //check for language prefix:
  if (_data.size() >= 3) {
    if (_data[2] == '\r') {
      GSM7_Language::ISOLanguageUId_e
        langId = GSM7_Language::str2isoId((const char*)_data.get());
      if (langId != GSM7_Language::isoUnspecified) {
        _dCS = 0x10; //packed Gsm7-bit, language prefixed
        return;
      }
    }
  }
  _dCS = 0x0F; //packed Gsm7-bit, arbitrary language
}

void USSDataString::setDataKind(DataKind_e use_kind, uint8_t use_dcs/* = 0xFF*/)
{
  _dKind = use_kind;
  if (isPackedDCS()) {
    _dCS = use_dcs;
  } else if (isLatin1Text()) {
    setEOS();
    setLanguageDCS();
  } else if (isUCS2Text()) {
    _dCS = 0x48; //general Data Coding, UCS2
  } else
    _dCS = 0xFF;
}


//assigns USS data, that is plain LATIN1 text, determining language prefix
//if it's present.
void USSDataString::setText(const char * use_text, uint16_t txt_len/* = 0*/)
{
  _dKind = ussdLATIN1_TEXT;
  if (!txt_len)
    txt_len = (uint16_t)strlen(use_text);
  _data.clear();
  if (txt_len)
    _data.append((const uint8_t*)use_text, txt_len);
  setEOS(); //add terminating zero if neccesary
  //check for language prefix:
  setLanguageDCS();
}

//assigns USS data, converting it to plain LATIN1 text if possible.
//Returns true on success.
bool USSDataString::setText(const USSDataString & ussd_val)
{
  if (ussd_val.isLatin1Text()) {
    setText(ussd_val.getText(), ussd_val.length());
    return true;
  }
  uint16_t dLen = 0;
  if (ussd_val.isDataAsLatin1(&dLen)) {
    _dKind = ussdLATIN1_TEXT;
    _data.resize(dLen);
    dLen = ussd_val.getDataAsLatin1((char*)_data.getBuf());
    _data.resize(dLen); //padding may be cutted out
    setEOS();
    //check for language prefix:
    setLanguageDCS();
    return true;
  }
  return false;
}

//assings UCS2 data, that is UCS2 text
void USSDataString::setUCS2Text(const uint8_t * ucs2_text, uint16_t data_octs)
{
  _dKind = ussdUCS2_TEXT; _dCS = 0x48; //general Data Coding, UCS2
  _data.clear();
  _data.append(ucs2_text, data_octs);
}

//assigns USS data, converting it to UCS2 text if possible.
//Returns true on success.
bool USSDataString::setUCS2Text(const USSDataString & ussd_val)
{
  if (ussd_val.isUCS2Text()) {
    setUCS2Text(ussd_val.getData(), ussd_val.length());
    return true;
  }
  uint16_t dLen = 0;
  if (ussd_val.isDataAsUCS2(&dLen)) {
    _dKind = ussdUCS2_TEXT; _dCS = 0x48; //general Data Coding, UCS2
    _data.resize(dLen);
    dLen = ussd_val.getDataAsUCS2(_data.getBuf());
    _data.resize(dLen); //Note: language prefix may be excluded!
    return true;
  }
  return false;
}

//assigns USS data encoded according to CBS coding scheme (UCS2, GSM 7bit, etc)
void USSDataString::setData(uint8_t use_dcs, const uint8_t * uss_data, uint16_t data_octs)
{
  _dKind = ussdPackedDCS; _dCS = use_dcs;
  _data.clear();
  _data.append(uss_data, data_octs);
}

} //comp
} //ussman
} //smsc

