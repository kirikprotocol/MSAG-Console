/* ************************************************************************** *
 * Transaction Capabilities Provider interface.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVIDER_IFACE_HPP
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVIDER_IFACE_HPP

# include "eyeline/tcap/TCProviderCfg.hpp"
# include "eyeline/tcap/TDlgHandlerIfaceFactory.hpp"
# include "eyeline/tcap/TcapApi.hpp"

namespace eyeline {
namespace tcap {

class TCProviderIface {
protected:
  virtual ~TCProviderIface() {}

public:
  virtual bool init(const TCProviderCfg & use_cfg) = 0;
  //
  virtual void shutdown(void) = 0;
  //Registers factory of handlers of TCAP dialogue associated with given
  //application context.
  //Returns false if ApplicationContext with given OID is already registered
  //NOTE.1: 'Dialogue expiration timeout' shouldn't be less than
  //        Remote Operations timeout implied by application context used.
  // 
  //NOTE.2: If 'Dialogue expiration timeout' isn't set, the default value
  //        specified in TCProvider configuration is used.
  virtual bool registerAppCTX(const asn1::EncodedOID & ctx_oid,
                              TDlgHandlerIfaceFactory * dlg_hndls_factory,
                              TDlgTimeout dlg_exp_tmo = 0) /*throw()*/ = 0;
  //
  virtual void  unregisterAppCTX(const asn1::EncodedOID & ctx_oid) /*throw()*/ = 0;
  //
  virtual TcapApi * getTcapApi(void) /*throw()*/ = 0;
};


class TCProviderAC : public TCProviderIface {
public:
  virtual ~TCProviderAC() {}
};

}}

#endif /* __EYELINE_TCAP_PROVIDER_IFACE_HPP */

