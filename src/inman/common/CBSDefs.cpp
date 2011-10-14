#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/common/CBSDefs.hpp"

namespace smsc {
namespace cbs {

const ISO_LANG  GSM7_Language::_langCG0[16] = {
/* 0000 German       */ ISO_LANG("DE"),
/* 0001 English      */ ISO_LANG("EN"),
/* 0010 Italian      */ ISO_LANG("IT"),
/* 0011 French       */ ISO_LANG("FR"),
/* 0100 Spanish      */ ISO_LANG("ES"),
/* 0101 Dutch        */ ISO_LANG("NL"),
/* 0110 Swedish      */ ISO_LANG("SV"),
/* 0111 Danish       */ ISO_LANG("DA"),
/* 1000 Portuguese   */ ISO_LANG("PT"),
/* 1001 Finnish      */ ISO_LANG("FI"),
/* 1010 Norwegian    */ ISO_LANG("NO"),
/* 1011 Greek        */ ISO_LANG("EL"),
/* 1100 Turkish      */ ISO_LANG("TR"),
/* 1101 Hungarian    */ ISO_LANG("HU"),
/* 1110 Polish       */ ISO_LANG("PL"),
/* 1111 unspecified  */ ISO_LANG()
};

const ISO_LANG  GSM7_Language::_langCG2[16] = {
/* 0000 Czech       */ ISO_LANG("CS"),
/* 0001 Hebrew      */ ISO_LANG("HE"),
/* 0010 Arabic      */ ISO_LANG("AR"),
/* 0011 Russian     */ ISO_LANG("RU"),
/* 0100 Icelandic   */ ISO_LANG("IS"),
/* 0101 unspecified */ ISO_LANG(),
/* 0110 unspecified */ ISO_LANG(),
/* 0111 unspecified */ ISO_LANG(),
/* 1000 unspecified */ ISO_LANG(),
/* 1001 unspecified */ ISO_LANG(),
/* 1010 unspecified */ ISO_LANG(),
/* 1011 unspecified */ ISO_LANG(),
/* 1100 unspecified */ ISO_LANG(),
/* 1101 unspecified */ ISO_LANG(),
/* 1110 unspecified */ ISO_LANG(),
/* 1111 unspecified */ ISO_LANG()
};

GSM7LanguageUId_e GSM7_Language::str2isoId(const char * lang_str)
{
  for (unsigned short i = 0; i < cg0Unspecified; ++i) {
    if ((_langCG0[i]._id[0] == lang_str[0]) && (_langCG0[i]._id[1] == lang_str[1]))
      return static_cast<ISOLanguageUId_e>(i);
  }
  for (unsigned short i = 0; i <= cg2Icelandic; ++i) {
    if ((_langCG2[i]._id[0] == lang_str[0]) && (_langCG2[i]._id[1] == lang_str[1]))
      return static_cast<ISOLanguageUId_e>(i + 0x0F);
  }
  return isoUnspecified;
}


static CBS_DCS::TextEncoding  _enc_enm[4] = {
  CBS_DCS::dcGSM7Bit, CBS_DCS::dcBINARY8, CBS_DCS::dcUCS2, CBS_DCS::dcReserved
};

#define BIT_SET(x) (1 << (x))

CBS_DCS::TextEncoding  parseCBS_DCS(unsigned char dcs, CBS_DCS & res)
{
  res.UDHind = res.msgClassDefined = res.compressed = false;
  res.lngPrefix = CBS_DCS::lngNone;
  res.language.clear();

  unsigned char codingGroup = (dcs >> 4) & 0x0F;
  unsigned char codingScheme = dcs & 0x0F;

  switch (codingGroup) {
  case 0x00: {
    res.encoding = CBS_DCS::dcGSM7Bit;
    res.language = GSM7_Language::_langCG0[codingScheme];
  } break;

  case 0x01: {
    if (!codingScheme) {
      res.encoding = CBS_DCS::dcGSM7Bit;
      res.lngPrefix = CBS_DCS::lng4GSM7Bit;
    } else if (codingScheme == 1) {
      res.encoding = CBS_DCS::dcUCS2;
      res.lngPrefix = CBS_DCS::lng4UCS2;
    } else 
      res.encoding = CBS_DCS::dcReserved;
  } break;

  case 0x02: {
    res.encoding = CBS_DCS::dcGSM7Bit;
    res.language = GSM7_Language::_langCG2[codingScheme];
  } break;

  case 0x03: { //Reserved, GSM 7 bit default
    res.encoding = CBS_DCS::dcGSM7Bit;
  } break;

  case 0x04: case 0x05: case 0x06: case 0x07: { //General Data Coding indication
    res.compressed = (dcs & BIT_SET(5)) ? true : false;
    res.msgClassDefined = (dcs & BIT_SET(4)) ? true : false;
    if (res.msgClassDefined)
      res.msgClass = (dcs & 0x03);
    res.encoding = _enc_enm[((dcs >> 2) & 0x03)];
  } break;

  case 0x09: { //Message with User Data Header (UDH) structure:
    res.UDHind = res.msgClassDefined = true;
    res.msgClass = (dcs & 0x03);
    res.encoding = _enc_enm[((dcs >> 2) & 0x03)];
  } break;

  case 0x08: case 0x0A: case 0x0B: case 0x0C: case 0x0D: { //reserved
    res.encoding = CBS_DCS::dcReserved;
  } break;

  case 0x0E: { //WAP defined
    res.encoding = CBS_DCS::dcReserved;
  } break;

  case 0x0F: { //Data coding / message handling
    res.msgClassDefined = true;
    res.msgClass = (dcs & 0x03);
    res.encoding = _enc_enm[(dcs >> 2) & 0x01];
  } break;

//  case 0x08: case 0x0A: case 0x0B: case 0x0C: case 0x0D: //reserved
//  case 0x0E: //WAP defined 
  default:
    res.encoding = CBS_DCS::dcReserved;
  }
  return res.encoding;
}

} //cbs
}//smsc

