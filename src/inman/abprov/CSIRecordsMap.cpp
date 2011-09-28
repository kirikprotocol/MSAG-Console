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
//Returns true if record is overwritten
CSIRecordsMap::InsertResult
  CSIRecordsMap::insert(const CSIRecord & use_rec) /*throw()*/
{
  if ((unsigned)use_rec.csiId > mIdxMap.maxPos())
    return InsertResult(0, false);
  if (use_rec.csiId >= mIdxMap.size())
    mIdxMap.resize(use_rec.csiId + 1);
  if (mIdxMap[use_rec.csiId]) {
    CSIRecord & rRcd = mRcdArr[mIdxMap[use_rec.csiId] - 1];
    rRcd = use_rec;
    return InsertResult(&rRcd, true);
  }
  mRcdArr.append(use_rec);
  mIdxMap[use_rec.csiId] = mRcdArr.size();
  CSIRecord & rRcd = mRcdArr.atLast();
  return InsertResult(&rRcd, false);
}

CSIRecordsMap::size_type
  CSIRecordsMap::insert(const CSIRecordsMap & use_map) /*throw()*/
{
  size_type res = 0;
  const CSIRecord * pRcd = use_map.begin();
  while (pRcd) {
    if (insert(*pRcd).mIsOvr)
      ++res;
    pRcd = use_map.next(pRcd->csiId);
  }
  return res;
}

//Returns initialized/assigned element of map associated wit given key.
//if target element isn't assigned yet, it's initialized by default value.
//NOTE: throws if requested CSIUid_e if beyond the allowed range.
CSIRecord & CSIRecordsMap::at(CSIUid_e csi_id) /*throw()*/
{
  if ((unsigned)csi_id > mIdxMap.maxPos())
    throw smsc::util::Exception("CSIRecordsMap: uid=%u is out of range [0 .. %u]",
                                (unsigned)csi_id, (unsigned)mIdxMap.maxPos());

  if (csi_id >= mIdxMap.size())
    mIdxMap.resize(csi_id + 1);
  if (mIdxMap[csi_id])
    return mRcdArr[mIdxMap[csi_id] - 1];

  mRcdArr.append(CSIRecord(csi_id));
  mIdxMap[csi_id] = mRcdArr.size();
  return mRcdArr.atLast();
}

//Returns first assigned CSI record.
const CSIRecord * CSIRecordsMap::begin(void) const
{
  size_type curIdx = 0;
  while (curIdx < mIdxMap.size()) {
    if (mIdxMap[curIdx])
      return &(mRcdArr[mIdxMap[curIdx] - 1]);
    ++curIdx;
  }
  return 0;
}

//Returns assigned CSI record with id next to given one.
const CSIRecord * CSIRecordsMap::next(CSIUid_e csi_id) const
{
  if ((unsigned)csi_id < mIdxMap.size()) {
    size_type nextIdx = csi_id;
    while (++nextIdx < mIdxMap.size()) {
      if (mIdxMap[nextIdx])
        return &(mRcdArr[mIdxMap[nextIdx] - 1]);
    }
  }
  return 0;
}


CSIRecordString_t CSIRecordsMap::toString(CSIUid_e csi_id) const
{
  CSIRecordString_t rval;
  const CSIRecord * pRcd = find(csi_id);

  if (pRcd) {
    pRcd->toString(rval);
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
  std::string str;
  str.reserve(str_size());
    
  str = "SCFs: ";
  if (empty())
    str += "<none>";
  else {
    const CSIRecord * pRcd = begin();
    do {
      CSIRecordString_t rcd;

      pRcd->toString(rcd);
      str += rcd.c_str();
      if ((pRcd = next(pRcd->csiId)))
        str += ", ";
    } while (pRcd);
  }
  return str;
}

} //iaprvd
} //inman
} //smsc


