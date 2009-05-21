#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/provd/TDlgReqSender.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

/* ************************************************************************* *
 * class TDlgRequestSenderAC implementation
 * ************************************************************************* */
TReqSendResult
TDlgRequestSenderAC::sendMessage(SuaApi * sua_iface) const
{
  SuaApi::CallResult rc = sua_iface->unitdata_req(_udt.userData(), _udt.userDataLen(),
                                                  _udt.calledAddr(), _udt.calledAddrLen(),
                                                  _udt.callingAddr(), _udt.callingAddrLen(),
                                                  _udt.msgProperties());
  return TReqSendResult(( rc.operationResult == SuaApi::OK ) ?
                        TReqSendResult::SEND_OK : TReqSendResult::SCCP_ERROR,
                        rc.suaConnectNum);
}

TReqSendResult
TDlgRequestSenderAC::sendMessage(SuaApi * sua_iface, unsigned int link_num) const
{
  SuaApi::CallResult rc = sua_iface->unitdata_req(_udt.userData(), _udt.userDataLen(),
                                                  _udt.calledAddr(), _udt.calledAddrLen(),
                                                  _udt.callingAddr(), _udt.callingAddrLen(),
                                                  _udt.msgProperties(), link_num);
  return TReqSendResult(( rc.operationResult == SuaApi::OK ) ?
                        TReqSendResult::SEND_OK : TReqSendResult::SCCP_ERROR,
                        rc.suaConnectNum);
}


/* ************************************************************************* *
 * class TBeginReqSender implementation
 * ************************************************************************* */
TDlgRequestSenderAC::SerializationResult_e
  TBeginReqSender::serialize2UDT(const SCCPAddress& src_addr,
                                 const SCCPAddress& dst_addr)
{
  //TODO: 
  return TDlgRequestSenderAC::srlzBadTransactionPortion;
}

/* ************************************************************************* *
 * class TContReqSender implementation
 * ************************************************************************* */
TDlgRequestSenderAC::SerializationResult_e
  TContReqSender::serialize2UDT(const SCCPAddress& src_addr,
                                const SCCPAddress& dst_addr)
{
  //TODO: 
  return TDlgRequestSenderAC::srlzBadTransactionPortion;
}

/* ************************************************************************* *
 * class TEndReqSender implementation
 * ************************************************************************* */
TDlgRequestSenderAC::SerializationResult_e
  TEndReqSender::serialize2UDT(const SCCPAddress& src_addr,
                               const SCCPAddress& dst_addr)
{
  //TODO: 
  return TDlgRequestSenderAC::srlzBadTransactionPortion;
}


/* ************************************************************************* *
 * class TPAbortReqSender implementation
 * ************************************************************************* */
TDlgRequestSenderAC::SerializationResult_e
  TPAbortReqSender::serialize2UDT(const SCCPAddress& src_addr,
                                  const SCCPAddress& dst_addr)
{
  //TODO: 
  return TDlgRequestSenderAC::srlzBadTransactionPortion;
}


/* ************************************************************************* *
 * class TUAbortReqSender implementation
 * ************************************************************************* */
TDlgRequestSenderAC::SerializationResult_e
  TUAbortReqSender::serialize2UDT(const SCCPAddress& src_addr,
                                  const SCCPAddress& dst_addr)
{
  //TODO: 
  return TDlgRequestSenderAC::srlzBadTransactionPortion;
}


} //provd
} //tcap
} //eyeline

