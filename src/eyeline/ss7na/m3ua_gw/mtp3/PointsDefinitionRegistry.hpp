#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_POINTSDEFINITIONREGISTRY_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_POINTSDEFINITIONREGISTRY_HPP__

# include <string>
# include <sys/types.h>

# include "logger/Logger.h"
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/ss7na/common/types.hpp"
# include "eyeline/utilx/ObjectRegistry.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {

struct PointInfo {
  PointInfo(uint8_t network_indicator, const std::string& a_standard)
  : ni(network_indicator) {
    if ( strcasecmp(a_standard.c_str(), "ITU") && strcasecmp(a_standard.c_str(), "ANSI") )
      throw smsc::util::Exception("PointInfo::PointInfo:::: invalid value='%s' for standard parameter - expected value is 'ANSI' or 'ITU'",
                                  a_standard.c_str());
    standard = a_standard;
  }
  uint8_t ni;
  std::string standard;
};

class PointsDefinitionRegistry : public utilx::Singleton<PointsDefinitionRegistry>,
                                 public utilx::ObjectRegistry<PointInfo, common::point_code_t,
                                                              smsc::core::synchronization::Mutex> {
public:
  void insert(const PointInfo& point_desc, const common::point_code_t lpc) {
    smsc_log_info(_logger, "PointsDefinitionRegistry::insert::: insert pointInfo={%u,%s} for lpc=%d",
                  point_desc.ni, point_desc.standard.c_str(), lpc);
    utilx::ObjectRegistry<PointInfo, common::point_code_t,
                          smsc::core::synchronization::Mutex>::insert(point_desc, lpc);
  }
private:
  PointsDefinitionRegistry()
  : _logger(smsc::logger::Logger::getInstance("mtp3"))
  {}
  friend class utilx::Singleton<PointsDefinitionRegistry>;

  smsc::logger::Logger* _logger;
};

}}}}

#endif
