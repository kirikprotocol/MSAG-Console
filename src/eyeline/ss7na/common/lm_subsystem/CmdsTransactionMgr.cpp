#include <utility>
#include <exception>
#include "CmdsTransactionMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace lm_subsystem {

void CmdsTransactionMgr::addOperation(LM_Command* cmd)
{
  smsc_log_debug(_logger, "CmdsTransactionMgr::addOperation::: cmd=0x%p",
                 cmd);
  _transactionCmds.push_back(cmd);
}

void CmdsTransactionMgr::addOperation(LM_Command* cmd,
                                      const std::string& cancelled_cmd_id)
{
  if ( cmd->getId() == "" )
    throw smsc::util::Exception("CmdsTransactionMgr::addOperation::: command id isn't set");

  smsc_log_debug(_logger, "CmdsTransactionMgr::addOperation::: cmd=0x%p, cancelled_cmd_id='%s'",
                 cmd, cancelled_cmd_id.c_str());
  cmd_ids_t::iterator iter = _cmdIds.find(cmd->getId());
  if ( iter != _cmdIds.end() ) {
    if ( cmd->replaceable() ) {
      smsc_log_debug(_logger, "CmdsTransactionMgr::addOperation::: delete replaceable 0x%p",
                     *(iter->second));
      delete *(iter->second);
      *(iter->second) = cmd;
    } else
      throw smsc::util::Exception("CmdsTransactionMgr::addOperation::: command with id='%s' already registered",
                                  cmd->getId().c_str());
  }

  iter = _cmdIds.find(cancelled_cmd_id);
  if ( iter != _cmdIds.end() ) {
    smsc_log_debug(_logger, "CmdsTransactionMgr::addOperation::: delete cancelled 0x%p",
                         *(iter->second));
    delete *(iter->second);
    _transactionCmds.erase(iter->second);
  }

  trn_cmds_t::iterator insIter = _transactionCmds.insert(_transactionCmds.end(), cmd);
  _cmdIds.insert(std::make_pair(cmd->getId(), insIter));
}

void CmdsTransactionMgr::commit()
{
  for (trn_cmds_t::iterator iter = _transactionCmds.begin(), end_iter = _transactionCmds.end();
       iter != end_iter; ++iter) {
    try {
      smsc_log_debug(_logger, "CmdsTransactionMgr::commit::: call updateConfiguration for object=0x%p", *iter);
      (*iter)->updateConfiguration();
      smsc_log_debug(_logger, "CmdsTransactionMgr::commit::: remove object=0x%p", *iter);
      delete *iter;
    } catch (std::exception& ex) {
      smsc_log_error(_logger, "CmdsTransactionMgr::commit::: caught exception='%s'; command processing failure, cmdId='%s'",
                     ex.what(), (*iter)->getId().c_str());
    }
  }
  _transactionCmds.clear();
  _cmdIds.clear();
}

void CmdsTransactionMgr::rollback() {
  while ( !_transactionCmds.empty() ) {
    delete _transactionCmds.front();
    _transactionCmds.pop_front();
  }
  _cmdIds.clear();
}

}}}}
