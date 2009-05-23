#ifndef __EYELINE_TCAP_PROVD_TCAPCONFIGURATION_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_TCAPCONFIGURATION_HPP__

# include "eyeline/sccp/SCCPAddress.hpp"
# include "eyeline/utilx/Singleton.hpp"

//TODO: get rid off ConfigView usage 
# include "util/config/ConfigView.h"

namespace eyeline {
namespace tcap {
namespace provd {

class TCAPConfiguration : public utilx::Singleton<TCAPConfiguration> {
public:
  void initialize(smsc::util::config::ConfigView& cfg);
  const sccp::SCCPAddress& getOwnAddress() const;
private:
  sccp::SCCPAddress _ownAddress;
};

}}}

#endif
