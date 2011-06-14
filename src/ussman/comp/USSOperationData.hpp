/* ************************************************************************** *
 * Remote USS Operation request/result data definition.
 * ************************************************************************** */
#ifndef __SMSC_USSMAN_USS_OPERATION_DATA
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_USSMAN_USS_OPERATION_DATA

#include "util/TonNpiAddress.hpp"
#include "ussman/comp/USSDataString.hpp"

namespace smsc {
namespace ussman {
namespace comp {

using smsc::util::TonNpiAddress;

struct USSOperationData {
  TonNpiAddress   _msIsdn;  //subcsriber ISDN address
  USSDataString   _ussData; //operation request/result data

  static const uint16_t _max_strSZ = TonNpiAddress::_strSZ + USSDataString::_max_StrForm_sz;

  USSOperationData()
  { }
  USSOperationData(const TonNpiAddress & ms_isdn, const USSDataString & uss_data)
    : _msIsdn(ms_isdn), _ussData(uss_data)
  { }
  ~USSOperationData()
  { }

  //Serializes USS Operation data to ASCIIZ-string storing it in provided buffer.
  //Note: it's a caller responsibility to provide necessary output buffer space
  //      (at maximum _max_strSZ).
  //Returns number of chars ought to be typed.
  //NOTE: if provided buffer length isn't enough, i.e (return_value >= buf_len),
  //      no output is performed.
  uint16_t serialize(char * use_buf, uint16_t buf_len = _max_strSZ) const
  {
    uint16_t res = (uint16_t)_msIsdn.toString(use_buf, buf_len);
    res += _ussData.toString(use_buf + res, buf_len - res);
    return res;
  }

  bool operator==(const USSOperationData & cmp_obj) const
  {
    return ((_msIsdn == cmp_obj._msIsdn) && (_ussData == cmp_obj._ussData));
  }

  bool operator<(const USSOperationData & cmp_obj) const
  {
    return (_msIsdn == cmp_obj._msIsdn) 
            ? (_ussData < cmp_obj._ussData) : (_msIsdn < cmp_obj._msIsdn);
  }
};

} //comp
} //ussman
} //smsc

#endif /* __SMSC_USSMAN_USS_OPERATION_DATA */

