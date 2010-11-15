#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/abprov/CSIRecordsMap.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
/* ************************************************************************** *
 * class CSIRecordsMap implementation:
 * ************************************************************************** */
CSIRecordString_t CSIRecordsMap::toString(CSIUid_e csi_id) const
{
  CSIRecordString_t rval;
  CSIRecordsMap::const_iterator cit = find(csi_id);

  if (cit != end()) {
    cit->second.toString(rval);
  } else {
    CSIRecord   empty;
    empty.csiId = csi_id;
    empty.toString(rval);
  }
  return rval;
}

size_t  CSIRecordsMap::str_size(void) const
{
  return sizeof("SCFs: ")
          + (empty() ? sizeof("<none>") : size()*CSIRecord::_strSZ);
}

std::string CSIRecordsMap::toString(void) const
{
  std::string str(0, str_size());
    
  str = "SCFs: ";
  if (empty())
    str += "<none>";
  else {
    CSIRecordString_t rcd;
    CSIRecordsMap::const_iterator it = begin();
    bool first = true;
    do {
      if (!first)
        str += ", ";
      else
        first = false;
      it->second.toString(rcd);
      str += rcd.c_str();
    } while ((++it) != end());
  }
  return str;
}

} //iaprvd
} //inman
} //smsc


