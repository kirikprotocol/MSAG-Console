#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_SCCPSUBSYSTEM_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_SCCPSUBSYSTEM_HPP__

# include <netinet/in.h>
# include <list>
# include <string>

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/utilx/ThreadSpecificData.hpp"
# include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"

# include "eyeline/ss7na/common/types.hpp"
# include "eyeline/ss7na/common/ApplicationSubsystem.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/router/GTTranslationTable.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {

class SccpSubsystem : public common::ApplicationSubsystem,
                      public utilx::Singleton<SccpSubsystem*> {
public:
  virtual void stop();
  virtual void initialize(utilx::runtime_cfg::RuntimeConfig& rconfig);

  void reinitConnectAcceptor();

  static std::string makeAddressFamilyPrefix(const std::string& gt_mask_value);

  void addSccpUserPolicy(const std::string& route_id,
                         const std::list<std::string>& sccp_users,
                         const std::string& traffic_mode);

  void addMTP3Policy(const std::string& route_id,
                     common::point_code_t lpc,
                     const std::list<common::point_code_t>& dpcs,
                     const std::string& traffic_mode);
private:
  SccpSubsystem();
  friend class utilx::Singleton<SccpSubsystem*>;

  void fillInTranslationTable(utilx::runtime_cfg::RuntimeConfig& rconfig);
  void processRouteInfoToSccpUser(utilx::runtime_cfg::CompositeParameter* translation_entry_param,
                                  router::GTTranslationTable* translation_table,
                                  utilx::runtime_cfg::CompositeParameter* sccp_users_param);
  void processRouteInfoToMTP3(utilx::runtime_cfg::CompositeParameter* translation_entry_param,
                              router::GTTranslationTable* translation_table);

  void initializeConnectAcceptor(const std::string& localHost,
                                 in_port_t localPort);

  void addSccpUserPolicy(const std::string& route_id,
                         utilx::runtime_cfg::CompositeParameter* sccp_users_param,
                         const std::string& traffic_mode);
  void addMTP3Policy(const std::string& route_id,
                     common::point_code_t lpc,
                     utilx::runtime_cfg::CompositeParameter* dpcs_param,
                     const std::string& traffic_mode);

  char _acceptorName[128];
};

}}}}

#endif
