#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/tests/AbonentsDB.hpp"

using smsc::core::synchronization::MutexGuard;

namespace smsc  {
namespace inman {
namespace test {

/* ************************************************************************** *
 * class AbonentsDB implementation:
 * ************************************************************************** */
void AbonentsDB::initDB(unsigned n_abn, const AbonentPreset * p_abn)
{
  for (unsigned i = 0; i < n_abn; ++i) {
    AbonentInfo  abn(p_abn[i].abType, p_abn[i].addr, p_abn[i].imsi);
    if (!abn.Empty())
      _registry.insert(AbonentsMAP::value_type(++_lastAbnId, abn));
  }
}

AbonentsDB * AbonentsDB::getInstance(void)
{
  static AbonentsDB _abnData;
  return &_abnData;
}

AbonentsDB * AbonentsDB::Init(unsigned n_abn, const AbonentPreset * p_abn)
{
  AbonentsDB * adb = AbonentsDB::getInstance();
  adb->initDB(n_abn, p_abn);
  return adb;
}

void AbonentsDB::printAbnInfo(FILE * stream, const AbonentInfo & abn, unsigned ab_id)
{
  fprintf(stream, "abn.%u: isdn <%s>, %s\n", ab_id,
          abn.msIsdn.length ? abn.msIsdn.toString().c_str() : " ",
          abn.toString().c_str());
}

AbonentInfo * AbonentsDB::getAbnInfo(unsigned ab_id)
{
  MutexGuard  grd(_sync);
  AbonentsMAP::iterator it = _registry.find(ab_id);
  return (it != _registry.end()) ? &(it->second) : NULL;
}

unsigned AbonentsDB::searchNextAbn(AbonentContract_e ab_type, unsigned min_id/* = 0*/) const
{
  MutexGuard  grd(_sync);
  AbonentsMAP::const_iterator it = _registry.begin();
  if (min_id && (min_id <= _registry.size())) {
    it = _registry.find(min_id);
  }
  for (; it != _registry.end(); ++it) {
    const AbonentInfo & abn = it->second;
    if (abn.abType == ab_type)
      return it->first;
  }
  return 0;
}

unsigned AbonentsDB::searchAbn(const std::string & addr) const
{
  TonNpiAddress   subscr;
  if (!subscr.fromText(addr.c_str()))
    return 0;
  return searchAbn(subscr);
}

unsigned AbonentsDB::searchAbn(const TonNpiAddress & subscr) const
{
  MutexGuard  grd(_sync);
  for (AbonentsMAP::const_iterator
       it = _registry.begin(); it != _registry.end(); ++it) {
    const AbonentInfo & abn = it->second;
    if (abn.msIsdn == subscr)
      return it->first;
  }
  return 0; //unknown
}


void AbonentsDB::printAbnInfo(FILE * stream, unsigned ab_id) const
{
  MutexGuard  grd(_sync);
  AbonentsMAP::const_iterator it = _registry.find(ab_id);
  if (it != _registry.end()) {
    const AbonentInfo & abn = it->second;
    AbonentsDB::printAbnInfo(stream, abn, ab_id);
  }
}

//0, 0 - print ALL
void AbonentsDB::printAbonents(FILE * stream, unsigned min_id/* = 0*/, unsigned max_id/* = 0*/) const
{
  MutexGuard  grd(_sync);
  if (!_registry.size())
    return;
  if (!min_id || (min_id > _registry.size()))
    min_id = 1;
  if (!max_id || (max_id > _registry.size()))
    max_id = (unsigned)_registry.size();

  AbonentsMAP::const_iterator it = _registry.find(min_id);
  while (min_id <= max_id) {
    const AbonentInfo & abn = it->second;
    printAbnInfo(stream, abn, min_id);
    ++min_id;
    ++it;
  }
}

unsigned AbonentsDB::setAbnInfo(const AbonentInfo & abn)
{
  unsigned ab_id = searchAbn(abn.msIsdn);
  MutexGuard  grd(_sync);
  if (!ab_id)
    ab_id = ++_lastAbnId; 
  _registry.insert(AbonentsMAP::value_type(ab_id, abn));
  return ab_id;
}

unsigned AbonentsDB::setAbnInfo(unsigned ab_id, const AbonentInfo & abn)
{
  MutexGuard  grd(_sync);
  if (ab_id <= _lastAbnId) {
    _registry.insert(AbonentsMAP::value_type(ab_id, abn));
    return ab_id;
  }
  return 0;
}

unsigned AbonentsDB::setAbonent(const TonNpiAddress & addr,
                      AbonentContract_e cntr_type/* = AbonentContractInfo::abtUnknown*/,
                      const char * sbcr_imsi/* = NULL*/)
{
  AbonentInfo   abn(addr, cntr_type, sbcr_imsi);
  unsigned      ab_id = searchAbn(abn.msIsdn);

  MutexGuard  grd(_sync);
  if (!ab_id)
    ab_id = ++_lastAbnId; 
  _registry.insert(AbonentsMAP::value_type(ab_id, abn));
  return ab_id;
}


} // test
} // namespace inman
} // namespace smsc

