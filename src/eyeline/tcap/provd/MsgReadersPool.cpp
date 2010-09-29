#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/provd/MsgReadersPool.hpp"
#include "eyeline/tcap/provd/MsgReader.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

void MsgReadersPool::init(ss7na::libsccp::SccpApi * sccp_api,
                 unsigned ini_num_readers, unsigned max_num_readers/* = 0*/)
{
  _iniReaders = ini_num_readers;
  _maxReaders = max_num_readers;
  _sccpApi = sccp_api;

  if (!_iniReaders) 
    _iniReaders = 2;
  if (_maxReaders)
    setMaxThreads((int)_maxReaders);
}

void MsgReadersPool::Start(void)
{
  while (_lastRdrId < _iniReaders)
    startTask(new MsgReader(_lastRdrId++, _sccpApi, _tIndProc));
}

void MsgReadersPool::Stop(bool do_wait/* = false*/)
{
  if (do_wait)
    shutdown(); //waits for threads (kills if necessary)
  else
    stopNotify();
  return;
}

bool MsgReadersPool::addMsgReader(void)
{
  if (_lastRdrId < _maxReaders) {
    startTask(new MsgReader(_lastRdrId++, _sccpApi, _tIndProc));
    return true;
  }
  return false;
}
}}}
