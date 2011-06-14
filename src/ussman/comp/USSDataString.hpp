/* ************************************************************************** *
 * Class helping to operate with various USS data strings.
 * ************************************************************************** */
#ifndef __SMSC_USSMAN_USSDATA_STR
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_USSMAN_USSDATA_STR

//#include <string>
#include "core/buffers/OctArrayT.hpp"

namespace smsc  {
namespace ussman {
namespace comp {

class USSDataString {
public:
  static const uint16_t _max_USSDStringLength = 160;  //maximum length of encoded/packed data (specified in
                                                      //3GPP TS 29.002 cl. 17.7.4 as MAP-SS-DataTypes.maxUSSD-StringLength)
  static const uint16_t _max_USSDLatin1Chars = 182;   //maximum number of Latin1 chars that may be set as USS data
  static const uint16_t _max_USSDUcs2Chars = 80;      //maximum number of UCS2 chars(two-bytes) that may be set as USS data

  enum DataKind_e {
    ussdPackedDCS = 0     //data is packed according to DCS
    , ussdLATIN1_TEXT = 1 //data is zero terminated string of LATIN1 chars
    , ussdUCS2_TEXT = 2   //data is vector of UCS2 chars
    , ussdUNKNOWN = 0xFF
  };

  typedef smsc::core::buffers::OctArray16_T<_max_USSDLatin1Chars + 2>
    DataArray_t;

protected:
  DataKind_e    _dKind;
  uint8_t       _dCS;   //meaningfull only in case of ussdPackedDCS
  DataArray_t   _data;  //Note: in case of LATIN1_TEXT stores also terminating zero char

  //Verifies that Latin1 data string has a terminating zero
  //Note: applicable only in case of ussdLATIN1_TEXT
  void setEOS(void);
  //checks Latin1 data string for language prefix and sets DCS accordingly.
  void setLanguageDCS(void);

public:
  //String representation forms are:
  //Latin1Text:  "asciiZ string"
  //UCS2Text:    "UCS2: 0xFFFF FFFF FFFF"
  //DCSpacked:   "dCS(FFh): 0xFFFFFFFFFFFFFFFFFFF"
  static const uint16_t _max_StrForm_sz = (uint16_t)(sizeof("dCS(FFh): 0x") + 160*3);

  USSDataString() : _dKind(ussdUNKNOWN), _dCS(0xFF)
  { }
  ~USSDataString()
  { }

  static DataKind_e uint2Kind(uint8_t ui_val)
  {
    return (ui_val <= ussdUCS2_TEXT) ? static_cast<DataKind_e>(ui_val) : ussdUNKNOWN;
  }

  DataKind_e getDataKind(void) const { return _dKind; }

  bool isPackedDCS(void) const { return (_dKind == ussdPackedDCS); }
  bool isLatin1Text(void) const { return (_dKind == ussdLATIN1_TEXT); }
  bool isUCS2Text(void) const { return (_dKind == ussdUCS2_TEXT); }

  uint8_t getDCS(void) const { return _dCS; }

  //Converts data to ASCIIZ-string representation storing it in provided buffer.
  //Note: it's a caller responsibility to provide necessary output buffer space
  //      (at maximimu _max_StrForm_sz).
  //Returns number of chars ought to be typed.
  //NOTE: if provided buffer length isn't enough, i.e (return_value >= buf_len),
  //      no output is performed.
  uint16_t toString(char * out_buf, uint16_t buf_len = _max_StrForm_sz) const;

  //Retuns lemgth of data in octets
  uint16_t length(void) const
  {
    return (_dKind != ussdLATIN1_TEXT) ? _data.size() : (_data.empty() ? 0 : _data.size() - 1);
  }
  //Returns data as zero terminated string of LATIN1 chars if possible.
  const char * getText(void) const
  {
    return (_dKind == ussdLATIN1_TEXT) ? (const char*)_data.get() : NULL;
  }

  const uint8_t * getData(void) const { return _data.get(); }

  //Checks if data may be represented as Latin1 text, if it's true and 
  //if 'p_len' argument is set, estimates resulted number of Latin1
  //chars (including language prefix).
  bool isDataAsLatin1(uint16_t * p_len = NULL) const;

  //Exports data as Latin1 text if possible.
  //Returns number of chars exported.
  //Note: it's a caller responsibility to provide necessary output buffer
  //      space. Usually, it doesn't exceed the _max_USSDLatin1Chars, but 
  //      it's safer to determine it by calling isDataAsLatin1().
  uint16_t getDataAsLatin1(char * out_str) const;

  //Exports data as Latin1 text if possible
  //bool getDataAsLatin1(std::string & out_str) const;

  //Checks if data may be represented as UCS2 text, if it's true and 
  //if 'p_len' argument is set, estimates resulted number of octets
  //(including language prefix).
  bool isDataAsUCS2(uint16_t * p_len = NULL) const;

  //Exports data as UCS2 text if possible.
  //Returns number of octets exported.
  //Note: it's a caller responsibility to provide necessary output buffer
  //      space. Usually, it doesn't exceed the 2*_max_USSDUcs2Chars, but 
  //      it's safer to determine it by calling isDataAsUCS2().
  uint16_t getDataAsUCS2(uint8_t * out_buf) const;

  //Checks if data is set and if 'p_len' argument is set, estimates resulted
  //number of octets in packed/encoded form.
  bool isDataAsPackedDCS(uint16_t * p_len = NULL) const;
  //Exports data as packed one according to DCS.
  //Returns number of octets exported.
  //Note: it's a caller responsibility to provide necessary output buffer
  //      space. Usually, it doesn't exceed the _max_USSDStringLength, but 
  //      it's safer to determine it by calling isDataAsPackedDCS().
  uint16_t getDataAsPackedDCS(uint8_t & out_dcs, uint8_t * out_buf) const;

  //Returns USS data buffer.
  const DataArray_t & getDataBuf(void) const { return _data; }

  // ---------------------------------------------------------------------

  //Exposes USS data buffer.
  DataArray_t & getDataBuf(void) { return _data; }
  //Sets data kind and DCS if data was previously assigned using getDataBuf().
  void setDataKind(DataKind_e use_kind, uint8_t use_dcs = 0xFF);

  //assigns USS data, that is plain LATIN1 text, determining language
  //prefix if it's present.
  void setText(const char * use_text, uint16_t txt_len = 0);
  //assigns USS data, converting it to plain LATIN1 text if possible.
  //Returns true on success.
  bool setText(const USSDataString & ussd_val);

  //assings USS data, that is UCS2 text
  void setUCS2Text(const uint8_t * ucs2_text, uint16_t data_octs);
  //assings USS data, converting it to UCS2 text  if possible.
  //Returns true on success.
  bool setUCS2Text(const USSDataString & ussd_val);

  //assigns USS data encoded according to CBS coding scheme (UCS2, GSM 7bit, etc)
  void setData(uint8_t use_dcs, const uint8_t * uss_data, uint16_t data_octs);

  bool operator==(const USSDataString & cmp_obj) const
  {
    return ((_dKind == cmp_obj._dKind) && (_dCS == cmp_obj._dCS)
            && (_data == cmp_obj._data));
  }

  bool operator<(const USSDataString & cmp_obj) const
  {
    if (_dKind == cmp_obj._dKind)
      return (_dCS == cmp_obj._dCS) ? (_data < cmp_obj._data) : (_dCS < cmp_obj._dCS);
    return (_dKind < cmp_obj._dKind);
  }
};

} //comp
} //ussman
} //smsc

#endif /* __SMSC_USSMAN_USSDATA_STR */
