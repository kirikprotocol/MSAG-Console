/* ************************************************************************** *
 * TCProvider: multithreaded processor of local TCAP dialogue indications 
 *             (TC_L_Cancel, TC_L_Reject, T_P_Abort, T_Notice).
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_TDLG_IND_GENERATOR_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROVD_TDLG_IND_GENERATOR_HPP

#include "core/threads/ThreadPool.hpp"

#include "eyeline/util/NCObjPoolT.hpp"
#include "eyeline/tcap/provd/TCLclIndications.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using smsc::core::synchronization::TimeSlice;

class TCLocalIndGenerator : protected TCLocalIndGeneratorIface,
                            protected smsc::core::threads::ThreadPool {
public:
  enum State_e { tpIdle = 0, tpRunning, tpStopping };

  TCLocalIndGenerator(TDlgFSMRegistry & dlg_reg, Logger * use_log = NULL)
    : _state(tpIdle), _iniThreads(2), _shdTmo(400, TimeSlice::tuMSecs)
    , _taskPool(1, /*erase_on_rlse =*/false), _dlgReg(dlg_reg), _logger(use_log)
  {
    if (!_logger)
      _logger = Logger::getInstance("tcap.provd");
  }
  virtual ~TCLocalIndGenerator()
  { }

  //NOTE: 'max_threads' = 0 means no limit
  void initThreads(unsigned ini_threads = 2, unsigned max_threads = 0)
  {
    if (max_threads)
      setMaxThreads((int)max_threads);
    if (ini_threads)
      _iniThreads = ini_threads;
    _taskPool.reserve(_iniThreads);
  }
  //
  void setShutdownTimeout(TimeSlice::UnitType_e time_unit, long use_tmo)
  {
    if (use_tmo)
      _shdTmo = TimeSlice(use_tmo, time_unit);
  }
  //
  void Start(void)
  {
    preCreateThreads(_iniThreads);
    _state = tpRunning;
  }
  //
  void Stop(bool do_wait = false)
  {
    _state = tpStopping;
    if (do_wait) { //wait for threads (kill if necessary)
      shutdown(_shdTmo.Units(), _shdTmo.Value()); 
      _state = tpIdle;
    } else
      stopNotify();
    return;
  }

  //Returns NULL if task pool capacity is exausted.
  TCIndLclCancel * allcLCancelInd(void)
  {
    TCLocalIndTask * pTask = allcInd();
    return pTask ? &(pTask->initLCancel()) : NULL;
  }
  TCIndLclReject * allcLRejectInd(void)
  {
    TCLocalIndTask * pTask = allcInd();
    return pTask ? &(pTask->initLReject()) : NULL;
  }
  TIndLclPAbort *  allcPAbortInd(void)
  {
    TCLocalIndTask * pTask = allcInd();
    return pTask ? &(pTask->initPAbort()) : NULL;
  }
  TIndLclNotice *  allcTNoticeInd(void)
  {
    TCLocalIndTask * pTask = allcInd();
    return pTask ? &(pTask->initNotice()) : NULL;
  }

private:
  typedef eyeline::util::NCOPool_T<TCLocalIndTask, uint32_t> TaskPool;

  volatile State_e  _state;
  unsigned          _iniThreads;
  TimeSlice         _shdTmo;      //shutdown timeout
  TaskPool          _taskPool;    //
  TDlgFSMRegistry  & _dlgReg;
  Logger *          _logger;

protected:
  // -------------------------------------------
  // -- TCLocalIndGeneratorIface implementation
  // -------------------------------------------
  //Returns NULL if task pool capacity is exausted.
  virtual TCLocalIndTask * allcInd(void)
  {
    TCLocalIndTask * pTask = _taskPool.allcObj();
    if (pTask)
      pTask->bind(*this, _dlgReg, _logger);
    return pTask;
  }
  virtual bool startInd(TCLocalIndTask * use_task)
  {
    if (_state == tpRunning) {
      startTask(use_task);
      return true;
    }
    return false;
  }
  virtual void rlseInd(TCLocalIndTask * use_task)
  {
    _taskPool.rlseObj(use_task);
  }
};

}}}

#endif /* __EYELINE_TCAP_PROVD_TDLG_IND_GENERATOR_HPP */

