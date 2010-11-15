/* ************************************************************************** *
 * INAP parameters for interaction with gsmSCF.
 * ************************************************************************** */
#ifndef __INMAN_ICS_IAPMGR_INSCF_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_IAPMGR_INSCF_HPP

#include "inman/common/RPCList.hpp"
#include "inman/services/iapmgr/SKAlgorithms.hpp"

namespace smsc  {
namespace inman {
namespace iapmgr {

using smsc::util::IMSIString;
using smsc::inman::common::RPCList;
using smsc::inman::common::RPCListATT;

/* ************************************************************************** *
 * gsmSCF interaction parameters used by CAP3SMS application context.
 * ************************************************************************** */
#define RP_MO_SM_transfer_rejected 21       //3GPP TS 24.011 Annex E-2
struct INParmsCapSms {
  //SwitchingCenter, SMS Center, INMan
  enum IDPLocationAddr_e { idpLiMSC = 0, idpLiSMSC = 1, idpLiSSF = 2 } ;
  enum IDPReqMode_e      { idpReqMT = 0, idpReqSEQ };

  IDPLocationAddr_e idpLiAddr;    //nature of initiator address to substitute into
                                  //LocationInformationMSC of InitialDP operation
                                  //while interacting this IN platfrom
  IDPReqMode_e      idpReqMode;   //mode of IDP requests (simultaneous or sequential)
                                  //this IN-point supports
  RPCList           rejectRPC;    //list of RP causes forcing charging denial because of low balance
  RPCListATT        retryRPC;     //list of RP causes indicating that IN point should be
                                  //interacted again a bit later

  INParmsCapSms()
    : idpLiAddr(idpLiMSC), idpReqMode(idpReqMT)
  {
    rejectRPC.push_back(RP_MO_SM_transfer_rejected);
  }
};

/* ************************************************************************** *
 * Complete gsmSCF interaction parameters.
 * ************************************************************************** */
class INScfCFG {
public:
  static const size_t _maxIdentSZ = 80;
  typedef smsc::core::buffers::FixedLengthString<_maxIdentSZ> IdentString_t;

  IdentString_t   _ident;     //INPlatform ident
  TonNpiAddress   _scfAdr;    //gsmSCF address always has ISDN international format
  SKAlgorithmsDb  _skDb;      //database of translation algoritms for various service keys 
  //optional params:
  IMSIString      _dfltImsi;  //default IMSI for abonents served by this gsmSCF
  INParmsCapSms   _capSms;    //Cap3Sms parameters


  explicit INScfCFG(const char * name = NULL) : _ident(name)
  { }
  ~INScfCFG()
  { }

  const char * Ident(void) const
  {
    return !_ident.empty() ? _ident.c_str() : _scfAdr.getSignals();
  }

  uint32_t getSKey(CSIUid_e tgt_csi, const CSIRecordsMap * org_csis) const
  {
    return _skDb.getSKey(tgt_csi, org_csis);
  }
};
typedef INScfCFG::IdentString_t INScfIdent_t;

} //iapmgr
} //inman
} //smsc

#endif /* __INMAN_ICS_IAPMGR_INSCF_HPP */

