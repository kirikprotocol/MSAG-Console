/* ************************************************************************** *
 * Cell Broadcast Data Coding related functions
 * ************************************************************************** */
#ifndef __INMAN_MAP_CBS_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_MAP_CBS_DEFS_HPP

namespace smsc {
namespace cbs {

struct ISO_LANG {
  char  _id[3]; //ISO 639, 2 letters

  ISO_LANG()
  {
    _id[0] = 0;
  }
  explicit ISO_LANG(const char * use_id)
  {
    if ((_id[0] = use_id[0]) != 0) {
      _id[1] = use_id[1]; _id[2] = 0;
    }
  }

  void clear(void)
  {
    _id[0] = _id[1] = _id[2] = 0;
  }
};


struct GSM7_Language {
  enum CodingGroupId_e {
    codingGroup0 = 0x0, codingGroup2 = 0x02
  };

  enum CG0_LangId_e { //languages of coding group 0 
    cg0German       = 0x00
  , cg0English      = 0x01
  , cg0Italian      = 0x02
  , cg0French       = 0x03
  , cg0Spanish      = 0x04
  , cg0Dutch        = 0x05
  , cg0Swedish      = 0x06
  , cg0Danish       = 0x07
  , cg0Portuguese   = 0x08
  , cg0Finnish      = 0x09
  , cg0Norwegian    = 0x0A
  , cg0Greek        = 0x0B
  , cg0Turkish      = 0x0C
  , cg0Hungarian    = 0x0D
  , cg0Polish       = 0x0E
  , cg0Unspecified  = 0x0F
  };

  enum CG2_LangId_e { //languages of coding group 2
    cg2Czech       = 0x00
  , cg2Hebrew      = 0x01
  , cg2Arabic      = 0x02
  , cg2Russian     = 0x03
  , cg2Icelandic   = 0x04
  , cg2Unspecified = 0x0F
  };

  enum ISOLanguageUId_e {
    //languages of coding group 0 
    isoGerman       = 0x00
  , isoEnglish      = 0x01
  , isoItalian      = 0x02
  , isoFrench       = 0x03
  , isoSpanish      = 0x04
  , isoDutch        = 0x05
  , isoSwedish      = 0x06
  , isoDanish       = 0x07
  , isoPortuguese   = 0x08
  , isoFinnish      = 0x09
  , isoNorwegian    = 0x0A
  , isoGreek        = 0x0B
  , isoTurkish      = 0x0C
  , isoHungarian    = 0x0D
  , isoPolish       = 0x0E
    //languages of coding group 2
  , isoCzech        = 0x0F
  , isoHebrew       = 0x10
  , isoArabic       = 0x11
  , isoRussian      = 0x12
  , isoIcelandic    = 0x13
  , isoUnspecified  = 0x1F
  };

  static const ISO_LANG  _langCG0[16];
  static const ISO_LANG  _langCG2[16];

  //
  static ISOLanguageUId_e str2isoId(const char * lang_str); //two-chars
  //
  static CodingGroupId_e isoId2CGId(ISOLanguageUId_e lang_id)
  {
    return ((lang_id <= isoPolish) || (lang_id == isoUnspecified))
            ? codingGroup0 : codingGroup2;
  }
  //
  static const ISO_LANG & isoId2str(ISOLanguageUId_e lang_id)
  {
    if (lang_id <= isoPolish)
      return _langCG0[lang_id];
    if (lang_id <= isoIcelandic)
      return _langCG2[lang_id - 0x0F];
    return _langCG0[cg0Unspecified];
  }
  //
  static const ISO_LANG & cg0Id2str(CG0_LangId_e lang_id)
  {
    return (lang_id <= cg0Unspecified) ? _langCG0[lang_id] : _langCG0[cg0Unspecified];
  }
  //
  static const ISO_LANG & cg2Id2str(CG2_LangId_e lang_id)
  {
    return (lang_id <= cg2Icelandic) ? _langCG2[lang_id] : _langCG2[cg2Unspecified];
  }
};
typedef GSM7_Language::ISOLanguageUId_e GSM7LanguageUId_e;

struct CBS_DCS {
  enum TextEncoding { dcGSM7Bit = 0, dcBINARY8 = 1, dcUCS2 = 2, dcReserved = 3};
  enum LangPrefix   { lngNone, lng4GSM7Bit, lng4UCS2 };

  TextEncoding  encoding;
  bool          compressed;
  bool          UDHind;
  LangPrefix    lngPrefix;
  bool          msgClassDefined;
  unsigned char msgClass : 2; //0 - 3
  ISO_LANG      language;
};

CBS_DCS::TextEncoding  parseCBS_DCS(unsigned char dcs, CBS_DCS & res);

inline CBS_DCS::TextEncoding  parseCBS_DCS(unsigned char dcs)
{
  CBS_DCS res;
  return parseCBS_DCS(dcs, res);
}

} //cbs
} //smsc
#endif /* __INMAN_MAP_CBS_DEFS_HPP */

