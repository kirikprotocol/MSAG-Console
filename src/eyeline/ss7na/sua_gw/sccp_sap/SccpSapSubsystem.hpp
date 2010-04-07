#ifndef __EYELINE_SS7NA_SUAGW_SCCPSAP_SCCPSAPSUBSYSTEM_HPP__
# define __EYELINE_SS7NA_SUAGW_SCCPSAP_SCCPSAPSUBSYSTEM_HPP__

# include <netinet/in.h>
# include <string>

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/utilx/ThreadSpecificData.hpp"
# include "eyeline/utilx/runtime_cfg/ParameterObserver.hpp"
# include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"

# include "eyeline/ss7na/common/ApplicationSubsystem.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sccp_sap {

class SccpSapSubsystem : public common::ApplicationSubsystem,
                         public utilx::runtime_cfg::ParameterObserver,
                         public utilx::Singleton<SccpSapSubsystem*> {
public:
  SccpSapSubsystem();
  virtual void stop();
  virtual void initialize(utilx::runtime_cfg::RuntimeConfig& rconfig);

  using ParameterObserver::addParameterEventHandler;
  using ParameterObserver::changeParameterEventHandler;
  //  using ParameterObserver::removeParameterEventHandler;

  virtual void addParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                        utilx::runtime_cfg::Parameter* added_parameter);
  virtual void changeParameterEventHandler(const utilx::runtime_cfg::CompositeParameter& context,
                                           const utilx::runtime_cfg::Parameter& modified_parameter);

private:
  void applyParametersChange();
  void initializeConnectAcceptor(const std::string& localHost,
                                 in_port_t localPort);

  static utilx::ThreadSpecificData<bool> _threadScopeModificationFlag;

  char _acceptorName[128];
};

}}}}

#endif
