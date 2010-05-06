#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMPOINTSADDPOINT_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMPOINTSADDPOINT_HPP__

# include <sys/errno.h>

# include "eyeline/ss7na/common/types.hpp"
# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/Exception.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"
# include "eyeline/ss7na/m3ua_gw/types.hpp"
# include "eyeline/utilx/strtol.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_Points_AddPoint : public common::lm_subsystem::LM_Command {
public:
  LM_Points_AddPoint(const std::string& point_name, const std::string& point_code,
                     const std::string& ni, const std::string& standard,
                     common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
  : _trnMgr(trn_mgr), _pointName(point_name), _pointCode(0),
    _ni(0), _standard(standard)
  {
    if ( _standard != "itu" && _standard != "ansi" )
      throw  common::lm_subsystem::InvalidCommandLineException("LM_Points_AddPoint::LM_Points_AddPoint::: invalid value for standard='%s'",
                                                               _standard.c_str());

    _pointCode = static_cast<common::point_code_t>(utilx::strtol(point_code.c_str(), NULL , 10));
    if ( !_pointCode && errno )
      throw common::lm_subsystem::InvalidCommandLineException("LM_Points_AddPoint::LM_Points_AddPoint::: invalid value for point_code='%s'",
                                                              point_code.c_str());

    _ni = static_cast<uint8_t>(utilx::strtol(ni.c_str(), NULL , 10));
    if ( !_ni && errno )
      throw common::lm_subsystem::InvalidCommandLineException("LM_Points_AddPoint::LM_Points_AddPoint::: invalid value for ni='%s'",
                                                              ni.c_str());

    if ( _ni > RESERVE_NATIONAL_NETWORK )
      throw common::lm_subsystem::InvalidCommandLineException("LM_Points_AddPoint::LM_Points_AddPoint::: value for ni='%s' is not in range [0..3]",
                                                              ni.c_str());

    setId(composeCommandId(_pointName));
  }

  virtual std::string executeCommand();

  virtual void updateConfiguration();

  static std::string composeCommandId(const std::string& point_name);

private:
  common::lm_subsystem::CmdsTransactionMgr& _trnMgr;
  const std::string& _pointName, _standard;
  common::point_code_t _pointCode;
  uint8_t _ni;
};

}}}}}

#endif
