#ifndef __EYELINE_SS7NA_M3UAGW_MTP3_MSUPROCESSOR_DISTRIBUTOR_HPP__
# define __EYELINE_SS7NA_M3UAGW_MTP3_MSUPROCESSOR_DISTRIBUTOR_HPP__

# include "logger/Logger.h"
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/m3ua_stack/messages/M3uaTLV.hpp"
# include "eyeline/ss7na/m3ua_gw/mtp3/sap/MTPSapInstance.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace mtp3 {
namespace msu_processor {

class Distributor : public utilx::Singleton<Distributor> {
public:
  void dispatch(const m3ua_stack::messages::TLV_ProtocolData& protocol_data) const;

protected:
  Distributor()
  : _logger(smsc::logger::Logger::getInstance("msu_proc")),
    _mtp3SapInstance(sap::MTPSapInstance::getInstance())
  {}
  friend class utilx::Singleton<Distributor>;

private:
  smsc::logger::Logger* _logger;
  sap::MTPSapInstance& _mtp3SapInstance;
};

}}}}}

#endif
