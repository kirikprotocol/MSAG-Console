#include "Region.hpp"
#include <sstream>

namespace smsc {
namespace util {
namespace config {
namespace region {

Region::Region(const std::string& id, const std::string& name, ulong_t bandwidth,const std::string& email, subjects_ids_lst_t subjects_ids, const std::string& infosmeSmscId)
  :_id(id), _name(name), _bandwidth(bandwidth), _email(email),_subjects_ids(subjects_ids), _subjectHasBeenExpanded(false),
   _infosmeSmscId(infosmeSmscId) {}

Region::MasksIterator
Region::getMasksIterator() {
  return MasksIterator(*this);
}

std::string
Region::toString() const {
  std::ostringstream obuf;
  obuf << "id=[" << _id
       << "],name=[" << _name
       <<"],bandwidth=[" << _bandwidth
       <<"]";
  if ( _subjectHasBeenExpanded ) {
    obuf << ",subjects=[";
    for (subjects_t::const_iterator iter=_subjects.begin(), end_iter=_subjects.end();
         iter != end_iter; ++iter) {
      smsc::util::config::route::MaskVector masksVec = iter->getMasks();
      std::string masksString;
      for(smsc::util::config::route::MaskVector::const_iterator m_iter = masksVec.begin(), m_end_iter = masksVec.end(); m_iter != m_end_iter; ++m_iter) {
        masksString += *m_iter;
      }
      obuf << "[id=[" << iter->getIdString() << "],mask=[" << masksString << "]], ";

    }
  } else {
    obuf << ",subject_ids=[";
    for (subjects_ids_lst_t::const_iterator iter=_subjects_ids.begin(), end_iter=_subjects_ids.end();
         iter != end_iter; ++iter)
      obuf << *iter << ", ";
  }
  return obuf.str();
}

void
Region::expandSubjectRefs(smsc::util::config::route::RouteConfig& routeCfg) {
  subjects_ids_lst_t::iterator iter = _subjects_ids.begin();
  subjects_ids_lst_t::iterator iter_end = _subjects_ids.end();
  while (iter != iter_end) {
    _subjects.push_back(routeCfg.getSubject((*iter).c_str()));
    ++iter;
  }
  _subjectHasBeenExpanded = true;
}


Region::MasksIterator::MasksIterator(Region& region) {
  for (subjects_t::const_iterator iter=region._subjects.begin(), end_iter=region._subjects.end();
       iter != end_iter; ++iter) {
    smsc::util::config::route::MaskVector masksVec = iter->getMasks();
    for(smsc::util::config::route::MaskVector::const_iterator m_iter = masksVec.begin(), m_end_iter = masksVec.end(); m_iter != m_end_iter; ++m_iter)
      _masks.push_back(*m_iter);
  }
  _iter = _masks.begin(); _end = _masks.end();
}

bool
Region::MasksIterator::fetchNext(std::string& mask)
{
  if (hasRecord()) {
    mask = *_iter;
    ++_iter;
    return true;
  } else
    return false;
}

bool
Region::MasksIterator::hasRecord() {return _iter != _end;}

}}}}
