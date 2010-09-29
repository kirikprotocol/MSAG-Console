/* ************************************************************************** *
 * Transaction Capabilities Provider interface.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVIDER_IFACE_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif /* __GNUC__ */
#define __EYELINE_TCAP_PROVIDER_IFACE_HPP

#include "eyeline/tcap/TCProviderCfg.hpp"
#include "eyeline/tcap/TDlgHandlerFactoryIface.hpp"
#include "eyeline/tcap/TcapApi.hpp"

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
                              TDlgHandlerFactoryIface * dlg_hndls_factory,
                              TDlgTimeout dlg_exp_tmo = 0) /*throw()*/ = 0;
  //
  virtual void  unregisterAppCTX(const asn1::EncodedOID & ctx_oid) /*throw()*/ = 0;
  //

  //Creates TCAP dialogue maintaining structures and dialogue indications
  //handler associated with given Appication Context.
  //Returns empty dialogue properties in case of application context with
  //given OID isn't registered by TCProvider.
  //Throws if TCAP dialog cann't be created due to unexpected reason.
  //
  //NOTE.1: variant with 'init_opcode' specified is associated with
  //         _ac_contextless_ops Appication Context.
  // 
  //NOTE.2: 'Dialogue expiration timeout' shouldn't be less than
  //        Remote Operations timeout implied by application context used.
  // 
  //NOTE.3: If 'Dialogue expiration timeout' isn't set, the default value
  //        specified in TCProvider configuration is used.
  virtual TDlgProperties createDialogue(uint8_t ssn_id, const asn1::EncodedOID & ac_oid,
                                   TDlgTimeout dlg_exp_tmo = 0) /*throw(std::exception)*/= 0;
  //
  virtual TDlgProperties createDialogue(uint8_t ssn_id, ros::LocalOpCode init_opcode,
                                   TDlgTimeout dlg_exp_tmo = 0) /*throw(std::exception)*/= 0;
};


class TCProviderAC : public TCProviderIface {
public:
  virtual ~TCProviderAC() {}
};

}}

#endif /* __EYELINE_TCAP_PROVIDER_IFACE_HPP */

