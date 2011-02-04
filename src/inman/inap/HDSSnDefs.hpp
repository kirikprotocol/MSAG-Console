/* ************************************************************************** *
 * SubSystem descriptive structures (supports HD version of TCAP BE).
 * ************************************************************************** */
#ifndef __SMSC_INMAN_INAP_SSN_DEFS__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_INAP_SSN_DEFS__

#include <set>
#include <map>

#include "core/synchronization/Event.hpp"

#include "inman/inap/SS7Types.hpp"
#include "inman/inap/SS7UnitsDefs.hpp"

namespace smsc {
namespace inman {
namespace inap {

using smsc::core::synchronization::Event;


struct UNITStatus {
  //matches the SS7UnitInstance::ConnectStatus
  enum BindStatus_e { unitIdle = 0, unitError, unitAwaited, unitBound };

  const uint8_t   _instId;     //SS7 communication unit instanceId, [1..255]
  BindStatus_e    _bindStatus;

  explicit UNITStatus(uint8_t inst_id = 0, BindStatus_e inst_status = unitIdle)
    : _instId(inst_id), _bindStatus(inst_status)
  { }
  //
  explicit UNITStatus(const SS7UnitInstance & unit_inst)
    : _instId(unit_inst.instId)
    , _bindStatus((unit_inst.connStatus == SS7UnitInstance::uconnError) ? unitError : unitIdle)
  { }

  bool operator< (const UNITStatus & obj2) const
  {
    return (_instId < obj2._instId);
  }
};
typedef std::set<UNITStatus> UnitsStatus;


class UNITBinding : protected UNITStatus {
private:
  bool    _signaled;

public:
  uint16_t  _maxId;      //maximum dlgId for TCAP BE instance
  uint16_t  _lastDlgId;  //dlgId of last allocated dialog
  uint16_t  _numOfDlgs;  //number of active dialogs controlled by this unit

  explicit UNITBinding(uint8_t inst_id = 0, uint16_t max_id = 0)
    : UNITStatus(inst_id), _maxId(max_id), _lastDlgId(0), _numOfDlgs(0)
  { }
  explicit UNITBinding(const UNITStatus & unit_inst, uint16_t max_id = 0)
    : UNITStatus(unit_inst), _maxId(max_id), _lastDlgId(0), _numOfDlgs(0)
  { }

  const UNITStatus & getUnit(void) const { return *this; }
  //
  bool isStatus(UNITStatus::BindStatus_e use_status) const
  {
    return (_bindStatus == use_status);
  }
  //
  bool isSignaled(void) const { return _signaled; }
  //
  bool isSignaled(UNITStatus::BindStatus_e use_status) const
  {
    return (_signaled && (_bindStatus == use_status));
  }

  void Rise(UNITStatus::BindStatus_e use_status)
  {
    _signaled = true; _bindStatus = use_status;
  }
  //
  void Reset(UNITStatus::BindStatus_e use_status = UNITStatus::unitIdle)
  {
    _signaled = false; _bindStatus = use_status;
  }

  void setStatus(UNITStatus::BindStatus_e use_status) { _bindStatus = use_status; }
};

struct SSNBinding : public Event {
protected:
  typedef std::map<uint8_t /*instId*/, UNITBinding> UnitsBinding;

  uint16_t      _maxDlgNum;  //maximum number of dialogs per SSN
  UnitsBinding  _units;

public:
  //NOTE: that's a sorted enum !!!
  enum SSBindStatus { ssnIdle = 0, ssnError, ssnPartiallyBound, ssnBound };

  //NOTE: tcap_inst_ids cann't be empty!
  SSNBinding(const SS7UnitInstsMap & tcap_inst_ids, uint16_t max_dlg_num)
    : _maxDlgNum(max_dlg_num)
  {
    uint16_t maxId = max_dlg_num/(uint16_t)tcap_inst_ids.size();

    for (SS7UnitInstsMap::const_iterator
         cit = tcap_inst_ids.begin(); cit != tcap_inst_ids.end(); ++cit) {
      _units.insert(UnitsBinding::value_type(cit->first,
                                    UNITBinding(UNITStatus(cit->second), maxId)));
    }
    _units.begin()->second._maxId += max_dlg_num % tcap_inst_ids.size();
  }

  uint16_t maxDlgNum(void) const { return _maxDlgNum; }

  bool incMaxDlgNum(uint16_t max_dlg_num)
  {
    if (max_dlg_num > _maxDlgNum) {
      uint16_t maxId = max_dlg_num/(uint16_t)_units.size();

      _maxDlgNum = max_dlg_num;
      for (UnitsBinding::iterator cit = _units.begin(); cit != _units.end(); ++cit)
        cit->second._maxId = maxId;
      _units.begin()->second._maxId += max_dlg_num % (uint16_t)_units.size();
      return true;
    }
    return false;
  }
  //Returns true if at least one UNIT instanceId is present
  bool getUnitsStatus(UnitsStatus & unit_set) const
  {
    unit_set.clear();
    for (UnitsBinding::const_iterator cit = _units.begin(); cit != _units.end(); ++cit)
      unit_set.insert(cit->second.getUnit());
    return !unit_set.empty();
  }

  //Returns number of unbinded TCAP BE instances
  unsigned unbindedUnits(void) const
  {
    unsigned rval = 0;
    for (UnitsBinding::const_iterator cit = _units.begin(); cit != _units.end(); ++cit) {
      if (!cit->second.isStatus(UNITStatus::unitBound))
        ++rval;
    }
    return rval;
  }
  //Searches for bound TCAP BE with lesser number of dialogs
  //Returns NULL if no TCAP BE available at that time
  UNITBinding * getUnitForDialog(void)
  {
    uint16_t    numOfDlgs = (uint16_t)-1;
    UnitsBinding::iterator  uIt = _units.end();
    //NOTE: set cann't be empty!
    for (UnitsBinding::iterator cit = _units.begin(); cit != _units.end(); ++cit) {
      if (cit->second.isStatus(UNITStatus::unitBound)
          && (cit->second._numOfDlgs < cit->second._maxId)
          && (cit->second._numOfDlgs < numOfDlgs)) {
        uIt = cit;
        numOfDlgs = cit->second._numOfDlgs;
      }
    }
    return (uIt == _units.end()) ? NULL : &(uIt->second);
  }

  //
  UNITBinding * getUnit(uint8_t unit_inst_id)
  {
    UnitsBinding::iterator it = _units.find(unit_inst_id);
    return (it != _units.end()) ? &(it->second) : 0;
  }
  //
  const UNITBinding * findUnit(uint8_t unit_inst_id) const
  {
    UnitsBinding::const_iterator it = _units.find(unit_inst_id);
    return (it != _units.end()) ? &(it->second) : 0;
  }

  SSBindStatus getStatus(void) const
  {
    uint8_t mask = 0x00; //'0000 0,Bound,Error,Idle'
    
    for (UnitsBinding::const_iterator cit = _units.begin(); cit != _units.end(); ++cit) {
      if (cit->second.isStatus(UNITStatus::unitIdle))
        mask |= 0x01;
      else if (cit->second.isStatus(UNITStatus::unitError))
        mask |= 0x02;
      else if (cit->second.isStatus(UNITStatus::unitBound))
        mask |= 0x04;
    }
    if ((mask & 0x07) == 0x02)  //all units faced error
      return ssnError;
    if ((mask & 0x07) == 0x04)  //all units are bound
      return ssnBound;
    if (mask & 0x04)            //at least one unit is bound
      return ssnPartiallyBound;
//  if ((mask & 0x07) == 0x01)  //all units are idle
    return ssnIdle;
  }

  //returns false in case of unknown unit_ist_id
  bool setUnitStatus(uint8_t unit_inst_id, UNITStatus::BindStatus_e use_status)
  {
    UNITBinding * unb = getUnit(unit_inst_id);
    if (unb) {
      unb->setStatus(use_status);
      return true;
    }
    return false;
  }
  //returns false in case of unknown unit_ist_id
  bool isUnitSignaled(uint8_t unit_inst_id) const
  {
    const UNITBinding * unb = findUnit(unit_inst_id);
    return  unb ? unb->isSignaled() : false;
  }
  //
  bool isAllSignaled(void) const
  {
    //using just a linear search because of tiny number of elements
    for (UnitsBinding::const_iterator cit = _units.begin(); cit != _units.end(); ++cit) {
      if (!cit->second.isSignaled())
        return false;
    }
    return true;
  }
  //Returns false if no TCAP BE instance exists with given instId
  bool SignalUnit(uint8_t unit_inst_id, UNITStatus::BindStatus_e unit_status)
  {
    UNITBinding * unb = getUnit(unit_inst_id);
    if (!unb)
      return false;

    unb->Rise(unit_status);
    if (isAllSignaled())
      Signal(); //all units either are bound or faced an error
    return true;
  }
  //
  void ResetUnit(uint8_t unit_inst_id, UNITStatus::BindStatus_e use_status = UNITStatus::unitIdle)
  {
    UNITBinding * unb = getUnit(unit_inst_id);
    if (unb)
      unb->Reset(use_status);
  }
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_SSN_DEFS__ */

