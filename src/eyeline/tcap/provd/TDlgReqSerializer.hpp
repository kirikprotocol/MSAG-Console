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


//Basic abstract class for all requests serializer.
class TDlgRequestSerializerAC {
protected:
  // -----------------------------------------------
  // -- TDlgRequestSerializerAC interface methods
  // -----------------------------------------------
  virtual bool convert2UDT(SUAUnitdataReq & use_udt) const = 0;

public:
  TDlgRequestSerializerAC()
  { }
  virtual ~TDlgRequestSerializerAC()
  { }

  //TODO: 2nd parameter: connect_num, etc
  bool sendMessage(SuaApi * sua_iface) const
  {
    SUAUnitdataReq  udt;
    SuaApi::ErrorCode_e rc = SuaApi::OK;
    if (convert2UDT(udt)) {
      rc = sua_iface->unitdata_req(udt.userData(), udt.userDataLen(),
                                   udt.calledAddr(), udt.calledAddrLen(),
                                   udt.callingAddr(), udt.callingAddrLen(),
                                   udt.msgProperties(), 0 /*TODO: */);
    }
    return rc ? false : true;
  }
};

template <class _TArg /* pubic: TDialogueRequestPrimitive */>
class TDlgRequestSerializerT : public TDlgRequestSerializerAC {
protected:
  const _TArg & _tReq;

public:
  TDlgRequestSerializerT(const _TArg & use_treq)
    : _tReq(use_treq)
  { }
  virtual ~TDlgRequestSerializerT()
  { }
};

class TBeginReqSerializer : public TDlgRequestSerializerT<TC_Begin_Req> {
protected:
  // -----------------------------------------------
  // -- TDlgRequestSerializerAC interface methods
  // -----------------------------------------------
  bool convert2UDT(SUAUnitdataReq & use_udt) const;

public:
  TBeginReqSerializer(const TC_Begin_Req & use_treq)
    : TDlgRequestSerializerT<TC_Begin_Req>(use_treq)
  { }
};
//
class TContReqSerializer : public TDlgRequestSerializerT<TC_Cont_Req> {
protected:
  // -----------------------------------------------
  // -- TDlgRequestSerializerAC interface methods
  // -----------------------------------------------
  bool convert2UDT(SUAUnitdataReq & use_udt) const;

public:
  TContReqSerializer(const TC_Cont_Req & use_treq)
    : TDlgRequestSerializerT<TC_Cont_Req>(use_treq)
  { }

  //These methods only for 1st response to T_Begin_Ind
  void acceptDlgByProvider(void);
  void rejectDlgByProvider(AssociateSourceDiagnostic::DlgSrvProvider_e use_cause =
                        AssociateSourceDiagnostic::dsp_null);
  void setAppCtx(const EncodedOID & use_acid);
};
//
class TEndReqSerializer : public TDlgRequestSerializerT<TC_End_Req> {
protected:
  // -----------------------------------------------
  // -- TDlgRequestSerializerAC interface methods
  // -----------------------------------------------
  bool convert2UDT(SUAUnitdataReq & use_udt) const;

public:
  TEndReqSerializer(const TC_End_Req & use_treq)
    : TDlgRequestSerializerT<TC_End_Req>(use_treq)
  { }

  //These methods only for 1st response to T_Begin_Ind
  void acceptDlgByProvider(void);
  void rejectDlgByProvider(AssociateSourceDiagnostic::DlgSrvProvider_e use_cause =
                        AssociateSourceDiagnostic::dsp_null);
  void setAppCtx(const EncodedOID & use_acid);
};
//
class TPAbortReqSerializer : public TDlgRequestSerializerT<TC_PAbort_Req> {
protected:
  // -----------------------------------------------
  // -- TDlgRequestSerializerAC interface methods
  // -----------------------------------------------
  bool convert2UDT(SUAUnitdataReq & use_udt) const;

public:
  TPAbortReqSerializer(const TC_PAbort_Req & use_treq)
    : TDlgRequestSerializerT<TC_PAbort_Req>(use_treq)
  { }
};
//
class TUAbortReqSerializer : public TDlgRequestSerializerT<TC_UAbort_Req> {
protected:
  // -----------------------------------------------
  // -- TDlgRequestSerializerAC interface methods
  // -----------------------------------------------
  bool convert2UDT(SUAUnitdataReq & use_udt) const;

public:
  TUAbortReqSerializer(const TC_UAbort_Req & use_treq)
    : TDlgRequestSerializerT<TC_UAbort_Req>(use_treq)
  { }
};

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_REQUESTS_SERIALIZER_HPP */

