#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/provd/TDlgReqComposers.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

/* ************************************************************************* *
 * class TBeginReqComposer implementation
 * ************************************************************************* */
TDlgRequestComposerAC::SerializationResult_e
  TBeginReqComposer::serialize2UDT(SUAUnitdataReq & use_udt,
                                   const SCCPAddress& src_addr,
                                   const SCCPAddress& dst_addr) const
{
  //TODO: 
  return TDlgRequestComposerAC::srlzBadTransactionPortion;
}

/* ************************************************************************* *
 * class TContReqComposer implementation
 * ************************************************************************* */
TDlgRequestComposerAC::SerializationResult_e
  TContReqComposer::serialize2UDT(SUAUnitdataReq & use_udt,
                                  const SCCPAddress& src_addr,
                                  const SCCPAddress& dst_addr) const
{
  //TODO: 
  return TDlgRequestComposerAC::srlzBadTransactionPortion;
}

/* ************************************************************************* *
 * class TEndReqComposer implementation
 * ************************************************************************* */
TDlgRequestComposerAC::SerializationResult_e
  TEndReqComposer::serialize2UDT(SUAUnitdataReq & use_udt,
                                 const SCCPAddress& src_addr,
                                 const SCCPAddress& dst_addr) const
{
  //TODO: 
  return TDlgRequestComposerAC::srlzBadTransactionPortion;
}


/* ************************************************************************* *
 * class TPAbortReqComposer implementation
 * ************************************************************************* */
TDlgRequestComposerAC::SerializationResult_e
  TPAbortReqComposer::serialize2UDT(SUAUnitdataReq & use_udt,
                                    const SCCPAddress& src_addr,
                                    const SCCPAddress& dst_addr) const
{
  //TODO: 
  return TDlgRequestComposerAC::srlzBadTransactionPortion;
}


/* ************************************************************************* *
 * class TUAbortReqComposer implementation
 * ************************************************************************* */
TDlgRequestComposerAC::SerializationResult_e
  TUAbortReqComposer::serialize2UDT(SUAUnitdataReq & use_udt,
                                    const SCCPAddress& src_addr,
                                    const SCCPAddress& dst_addr) const
{
  //TODO: 
  return TDlgRequestComposerAC::srlzBadTransactionPortion;
}


} //provd
} //tcap
} //eyeline

