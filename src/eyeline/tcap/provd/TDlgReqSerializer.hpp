/* ************************************************************************** *
 * TCAP dialogue requests serializer.
 * ************************************************************************** */
#ifndef __ELC_TCAP_REQUESTS_SERIALIZER_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_REQUESTS_SERIALIZER_HPP

#include "eyeline/tcap/TDialogueRequestPrimitives.hpp"
#include "eyeline/tcap/provd/SUARequests.hpp"
#include "eyeline/sua/libsua/SuaApi.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using eyeline::sua::libsua::SuaApi;
using eyeline::tcap::proto::AssociateSourceDiagnostic;

//
class TC_PAbort_Req : public TDialogueRequestPrimitive {
public:
  TC_PAbort_Req()
    : TDialogueRequestPrimitive(TCAPMessage::t_abort)
  { }

  //TODO: setters for PAbortCause
};


template <class _TArg /* pubic: TDialogueRequestPrimitive */>
class TDlgRequestSerializerT {
protected:
  const _TArg & _tReq;

  bool convertTReq2UDT(const _TArg & use_treq,
                      SUAUnitdataReq & use_udt) const
  {
    //TODO: 
    return false;
  }

public:
  TDlgRequestSerializerT(const _TArg & use_treq)
    : _tReq(use_treq)
  { }
  ~TDlgRequestSerializerT()
  { }

  //TODO: 2nd parameter: connect_num, etc
  bool sendMessage(SuaApi * sua_iface) const
  {
    SUAUnitdataReq  udt;
    SuaApi::ErrorCode_e rc = SuaApi::OK;
    if (convertTReq2UDT(_tReq, udt)) {
      rc = sua_iface->unitdata_req(udt.userData(), udt.userDataLen(),
                                   udt.calledAddr(), udt.calledAddrLen(),
                                   udt.callingAddr(), udt.callingAddrLen(),
                                   udt.msgProperties(), 0 /*TODO: */);
    }
    return rc ? false : true;
  }

  //These methods only for 1st response to T_Begin_Ind
  void rejectDlgByProvider(AssociateSourceDiagnostic::DlgSrvProvider_e use_cause =
                        AssociateSourceDiagnostic::dsp_null)
  { /*TODO: */  }

};

class TBeginReqSerializer : public TDlgRequestSerializerT<TC_Begin_Req> {
public:
  TBeginReqSerializer(const TC_Begin_Req & use_treq)
    : TDlgRequestSerializerT<TC_Begin_Req>(use_treq)
  { }
};
//
class TContReqSerializer : public TDlgRequestSerializerT<TC_Cont_Req> {
public:
  TContReqSerializer(const TC_Cont_Req & use_treq)
    : TDlgRequestSerializerT<TC_Cont_Req>(use_treq)
  { }
};
//
class TEndReqSerializer : public TDlgRequestSerializerT<TC_End_Req> {
public:
  TEndReqSerializer(const TC_End_Req & use_treq)
    : TDlgRequestSerializerT<TC_End_Req>(use_treq)
  { }
};
//
class TPAbortReqSerializer : public TDlgRequestSerializerT<TC_PAbort_Req> {
public:
  TPAbortReqSerializer(const TC_PAbort_Req & use_treq)
    : TDlgRequestSerializerT<TC_PAbort_Req>(use_treq)
  { }
};
//
class TUAbortReqSerializer : public TDlgRequestSerializerT<TC_UAbort_Req> {
public:
  TUAbortReqSerializer(const TC_UAbort_Req & use_treq)
    : TDlgRequestSerializerT<TC_UAbort_Req>(use_treq)
  { }
};

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_REQUESTS_SERIALIZER_HPP */

