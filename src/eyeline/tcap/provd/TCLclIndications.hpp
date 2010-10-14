/* ************************************************************************** *
 * TCProvider: TCAP dialogue local indications 
 *             L_Cancel, L_Reject, P_Abort, T_Notice.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_LOCAL_INDICATIONS_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROVD_LOCAL_INDICATIONS_HPP

#include "logger/Logger.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/buffers/FixedLengthString.hpp"

#include "eyeline/tcap/provd/TCompIndComposers.hpp"
#include "eyeline/tcap/provd/TDlgIndComposers.hpp"
#include "eyeline/tcap/provd/TDlgFSMRegistry.hpp"


namespace eyeline {
namespace tcap {
namespace provd {

using smsc::logger::Logger;

class TCLocalIndTask;

class TCLocalIndGeneratorIface {
public:
  virtual TCLocalIndTask * allcInd(void) = 0;
  virtual bool startInd(TCLocalIndTask * use_task) = 0;
  virtual void rlseInd(TCLocalIndTask * use_task) = 0;
};

/* ********************************************************************** *
 * Locally generated Component SubLayer indications:
 * ********************************************************************** */
class TCIndLclCancel : public TC_LCancelIndComposer {
protected:
  TCLocalIndTask *  _ownTask;

public:
  TCIndLclCancel(TCLocalIndTask & use_task)
    : TC_LCancelIndComposer(), _ownTask(&use_task)
  { }
  ~TCIndLclCancel()
  { }

  void process(void);
};
/*
class TCIndLclReject : public TC_LRejectIndComposer {
protected:
  TCLocalIndTask *  _ownTask;

public:
  TCIndLclReject(TCLocalIndTask & use_task)
    : TC_LRejectIndComposer(), _ownTask(&use_task)
  { }
  ~TCIndLclReject()
  { }

  void process(void);
};
*/
/* ********************************************************************** *
 * Locally generated Transaction SubLayer indications:
 * ********************************************************************** */

class TIndLclPAbort : public TPAbortIndComposer {
protected:
  TCLocalIndTask *  _ownTask;

public:
  TIndLclPAbort(TCLocalIndTask & use_task)
    : TPAbortIndComposer(), _ownTask(&use_task)
  { }
  ~TIndLclPAbort()
  { }

  void process(void);
};

class TIndLclNotice : public TNoticeIndComposer {
protected:
  TCLocalIndTask *  _ownTask;
  UDTDataBuffer     _usrData;

public:
  TIndLclNotice(TCLocalIndTask & use_task)
    : TNoticeIndComposer(), _ownTask(&use_task)
  { }
  ~TIndLclNotice()
  { }

  void process(void);

  void setDataBuffer(const UDTDataBuffer & udt_buf)
  {
    _usrData = udt_buf;
    setUserData(_usrData.get(), _usrData.getDataSize());
  }
};

/* ********************************************************************** *
 * Threaded task processing Locally generated indications:
 * ********************************************************************** */
class TCLocalIndTask : public smsc::core::threads::ThreadedTask {
private:
  static const size_t _MAX_TASKNAME_LEN = sizeof("%s[%Xh]") + sizeof("TCLocalIndTask") + sizeof(TDialogueId)*3;
  typedef smsc::core::buffers::FixedLengthString<_MAX_TASKNAME_LEN> NameString_t;

  enum IndKind_e { indNone = 0, indTNotice, indPAbort, indLCancel/*, indLReject */};

  union {
    void *  _aligner;
/*
    uint8_t _buf[eyeline::util::MaxSizeOf4_T<
                  TCIndLclCancel, TCIndLclReject, TIndLclPAbort, TIndLclNotice
                  >::VALUE];
*/
    uint8_t _buf[eyeline::util::MaxSizeOf3_T<
                    TCIndLclCancel, TIndLclPAbort, TIndLclNotice
                  >::VALUE];
  } _memObj;

  union {
    void *            _any;
    TCIndLclCancel *  _lCancel;
//    TCIndLclReject *  _lReject;
    TIndLclPAbort *   _tPAbort;
    TIndLclNotice *   _tNotice;
  } _pInd;

  IndKind_e                   _kind;
  TCLocalIndGeneratorIface *  _owner;
  TDlgFSMRegistry  *          _dlgReg;
  NameString_t                _logId;
  Logger *                    _logger;

protected:
  static const char * nmKind(IndKind_e ind_kind);

  TDialogueId getDlgId(void) const;
  const char * nmKind(void) const { return nmKind(_kind); }
  void clear(void);

public:
  TCLocalIndTask() : ThreadedTask(false)
    , _kind(indNone), _owner(NULL), _dlgReg(NULL), _logger(NULL)
  {
    _pInd._any = _memObj._aligner = NULL;
  }
  //
  ~TCLocalIndTask()
  {
    clear();
  }

  void bind(TCLocalIndGeneratorIface & use_owner,
            TDlgFSMRegistry & dlg_reg, Logger * use_log)
  {
    _owner = &use_owner; _dlgReg = &dlg_reg; _logger = use_log;
  }

  void process(void) { _owner->startInd(this); }

  TCIndLclCancel & initLCancel(void);
  //
  //TCIndLclReject & initLReject(void);
  //
  TIndLclPAbort &  initPAbort(void);
  //
  TIndLclNotice &  initNotice(void);

  // ---------------------------------------------------------------------
  // -- ThreadedTask interface methods (called only after process() call)
  // ---------------------------------------------------------------------
  virtual int Execute(void);
  virtual const char* taskName(void)
  {
    if (_logId.empty())
      snprintf(_logId.str, sizeof(_logId.str), "%s[%Xh]", nmKind(), getDlgId());
    return _logId.c_str();
  }
  virtual void onRelease(void)
  {
    _owner->rlseInd(this);
    isReleased = true;
  }
};


}}}

#endif /* __EYELINE_TCAP_PROVD_LOCAL_INDICATIONS_HPP */

