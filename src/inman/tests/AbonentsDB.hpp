/* ************************************************************************** *
 * class AbonentsDB (singleton): AbonentInfo registry.
 * ************************************************************************** */
#ifndef __INMAN_TEST_ABONENTS_DB_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_TEST_ABONENTS_DB_HPP

#include "core/synchronization/Mutex.hpp"
#include "inman/tests/AbonentInfo.hpp"

namespace smsc  {
namespace inman {
namespace test {

using smsc::core::synchronization::Mutex;

struct AbonentPreset {
  AbonentContract_e  abType;
  const char *  addr;
  const char *  imsi;

  explicit AbonentPreset(AbonentContract_e cntr_type = AbonentContractInfo::abtUnknown,
                        const char * sbcr_addr = NULL, const char * sbcr_imsi = NULL)
    : abType(cntr_type), addr(sbcr_addr), imsi(sbcr_imsi)
  { }
};


class AbonentsDB { //Singleton
protected:
  typedef std::map<unsigned, AbonentInfo> AbonentsMAP;
  typedef std::map<TonNpiAddress, unsigned> AbonentsIdxMAP;

  mutable Mutex   _sync;
  AbonentsMAP     _registry;
  AbonentsIdxMAP  _idxReg;
  unsigned        _lastAbnId;

  void initDB(unsigned n_abn, const AbonentPreset * p_abn);

  void insertAbonent(unsigned ab_idx, const AbonentInfo & ab_info)
  {
    _registry[ab_idx] = ab_info;
    _idxReg[ab_info.msIsdn] = ab_idx;
  }

  AbonentsDB(void) : _lastAbnId(0)
  { }
  ~AbonentsDB()
  { }

public:
  static AbonentsDB * getInstance(void);
  static AbonentsDB * Init(unsigned n_abn, const AbonentPreset * p_abn);
  static void printAbnInfo(FILE * stream, const AbonentInfo & abn, unsigned ab_id);

  unsigned getMaxAbId(void) const { return _lastAbnId; }
  unsigned nextId(unsigned ab_id) const { return (ab_id >= _lastAbnId) ? 1 : ++ab_id; }

  AbonentInfo * getAbnInfo(unsigned ab_id);
  const AbonentInfo * getAbnInfo(unsigned ab_id) const;
  //
  unsigned searchNextAbn(AbonentContract_e ab_type, unsigned min_id = 0) const;
  //
  unsigned searchAbn(const std::string & addr) const;
  //
  unsigned searchAbn(const TonNpiAddress & subscr) const;

  //
  void printAbnInfo(FILE * stream, unsigned ab_id) const;
  //0, 0 - print ALL
  void printAbonents(FILE * stream, unsigned min_id = 0, unsigned max_id = 0) const;

  //Adds AbonentInfo entry
  unsigned setAbnInfo(const AbonentInfo & ab_info);
  //Overwrires/Adds AbonentInfo entry 
  unsigned setAbnInfo(unsigned ab_id, const AbonentInfo & ab_info);
  //Reserves AbonentInfo entry for abonent with given address
  unsigned setAbonent(const TonNpiAddress & addr,
                      AbonentContract_e cntr_type = AbonentContractInfo::abtUnknown,
                      const char * sbcr_imsi = NULL);
};

} //test
} //inman
} //smsc
#endif /* __INMAN_TEST_ABONENTS_DB_HPP */

