#ifndef __EYELINE_SS7NA_COMMON_LMSUBSYSTEM_CMDSTRANSACTIONMGR_HPP__
# define __EYELINE_SS7NA_COMMON_LMSUBSYSTEM_CMDSTRANSACTIONMGR_HPP__

# include <map>
# include <list>

# include "logger/Logger.h"
# include "eyeline/ss7na/common/lm_subsystem/types.hpp"
# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace lm_subsystem {

class CmdsTransactionMgr {
public:
  CmdsTransactionMgr()
  : _logger(smsc::logger::Logger::getInstance("lm_susbsys"))
  {}

  void addOperation(LM_Command* cmd);
  void addOperation(LM_Command* cmd, const std::string& cancelled_cmd_id);
  void commit();
  void rollback();

private:
  CmdsTransactionMgr(const CmdsTransactionMgr& rhs);
  CmdsTransactionMgr& operator=(const CmdsTransactionMgr& rhs);

  smsc::logger::Logger* _logger;
  typedef std::list<LM_Command*> trn_cmds_t;
  trn_cmds_t _transactionCmds;

  typedef std::map<std::string, trn_cmds_t::iterator> cmd_ids_t;
  cmd_ids_t _cmdIds;
};

}}}}

#endif
