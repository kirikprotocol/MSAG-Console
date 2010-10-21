/* ************************************************************************** *
 * TCAP dialogue indication composers (Transaction sub-layer indications).
 * ************************************************************************** */
#ifndef __ELC_TCAP_INDICATIONS_COMPOSERS_HPP
#ident "@(#)$Id$"
#define __ELC_TCAP_INDICATIONS_COMPOSERS_HPP

#include "eyeline/tcap/ContextlessOps.hpp"
#include "eyeline/tcap/TDlgIndicationPrimitives.hpp"
#include "eyeline/tcap/proto/TCMessage.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

template <class T_DLG_IND_Arg /* pubic: TDlgIndicationPrimitive */>
class TDlgIndicationComposerT : public T_DLG_IND_Arg {
protected:
  unsigned int _scspLink;

public:
  TDlgIndicationComposerT() : T_DLG_IND_Arg()
  { }
  ~TDlgIndicationComposerT()
  { }
  //
  T_DLG_IND_Arg & TInd(void) { return *this; }
  //
  const T_DLG_IND_Arg & TInd(void) const { return *this; }
  //
  void setDialogueId(const TDialogueId & use_id) { this->_dlgId = use_id; }
  //
  void setSCSPLink(unsigned int scsp_link_num) { _scspLink = scsp_link_num; }
  //Returns true on success
  bool setOrigAddress(const uint8_t * use_buf, unsigned buf_len)
  {
    return this->_orgAdr.unpackOcts(use_buf, buf_len) != 0;
  }
  //
  unsigned int getSCSPLink(void) const { return _scspLink; }
};

// -----------------------------------------------
// TR_BEGIN_IND: TR_Begin_Ind composer
// -----------------------------------------------
class TBeginIndComposer : public TDlgIndicationComposerT<TR_Begin_Ind> {
protected:
  uint32_t _orgTrId;

public:
  TBeginIndComposer() : TDlgIndicationComposerT<TR_Begin_Ind>()
  { }
  ~TBeginIndComposer()
  { }

  void init(proto::TMsgBegin & use_msg) /*throw(std::exception)*/;
  //
  bool setDestAddress(const uint8_t * use_buf, unsigned buf_len)
  {
    return _dstAdr.unpackOcts(use_buf, buf_len) != 0;
  }
  //
  uint32_t getOrigTrId(void) const { return _orgTrId; }
};

// -----------------------------------------------
// TR_CONT_IND: TR_Cont_Ind composer
// -----------------------------------------------
class TContIndComposer : public TDlgIndicationComposerT<TR_Cont_Ind> {
protected:
  uint32_t _orgTrId;
  uint32_t _dstTrId;

public:
  TContIndComposer() : TDlgIndicationComposerT<TR_Cont_Ind>()
  { }
  ~TContIndComposer()
  { }

  void init(proto::TMsgContinue & use_msg) /*throw(std::exception)*/;
  //
  uint32_t getOrigTrId(void) const { return _orgTrId; }
  //
  uint32_t getDestTrId(void) const { return _dstTrId; }
};

// -----------------------------------------------
// TR_END_IND: TR_End_Ind composer
// -----------------------------------------------
class TEndIndComposer : public TDlgIndicationComposerT<TR_End_Ind> {
protected:
  uint32_t _dstTrId;

public:
  TEndIndComposer() : TDlgIndicationComposerT<TR_End_Ind>()
  { }
  ~TEndIndComposer()
  { }
  //
  void init(proto::TMsgEnd & use_msg) /*throw(std::exception)*/;
  //
  uint32_t getDestTrId(void) const { return _dstTrId; }
};

// -----------------------------------------------
// TR_U_ABORT_IND: TR_UAbort_Ind composer
// -----------------------------------------------
class TUAbortIndComposer : public TDlgIndicationComposerT<TR_UAbort_Ind> {
protected:
  uint32_t _dstTrId;

public:
  TUAbortIndComposer() : TDlgIndicationComposerT<TR_UAbort_Ind>()
  { }
  ~TUAbortIndComposer()
  { }

  //NOTE: use_msg has _reason == u-abortCause
  void init(proto::TMsgAbort & use_msg)  /*throw(std::exception)*/;
  //
  uint32_t getDestTrId(void) const { return _dstTrId; }
};

// -----------------------------------------------
// TR_P_ABORT_IND: TR_PAbort_Ind composer
// -----------------------------------------------
class TPAbortIndComposer : public TDlgIndicationComposerT<TR_PAbort_Ind> {
protected:
  uint32_t _dstTrId;

public:
  TPAbortIndComposer() : TDlgIndicationComposerT<TR_PAbort_Ind>()
  { }
  ~TPAbortIndComposer()
  { }

  //Local TCProvider abort
  void init(uint32_t dst_trId, PAbort::Cause_t use_reason)
  {
    _dstTrId = dst_trId; _cause = use_reason;
  }
  //Remote TCProvider abort
  //NOTE: use_msg has _reason == p-abortCause
  void init(proto::TMsgAbort & use_msg)
  {
    _dstTrId = use_msg._dstTrId;
    _cause = *use_msg._reason.prvd().get();
  }
  //
  uint32_t getDestTrId(void) const { return _dstTrId; }
};

// -----------------------------------------------
// TR_NOTICE_IND: TR_Notice_Ind composer
// -----------------------------------------------
class TNoticeIndComposer : public TDlgIndicationComposerT<TR_Notice_Ind> {
public:
  TNoticeIndComposer() : TDlgIndicationComposerT<TR_Notice_Ind>()
  { }
  ~TNoticeIndComposer()
  { }
  //
  void setReturnCause(sccp::ReturnCause_t ret_err)
  {
    _cause = ret_err;
  }
  //
  bool setDestAddress(const uint8_t * dst_adr_buf, unsigned buf_len)
  {
    return _dstAdr.unpackOcts(dst_adr_buf, buf_len) != 0;
  }
  //
  void setUserData(const uint8_t * user_data, uint16_t data_len)
  {
    _usrDataLen = data_len; _usrDataBuf = user_data;
  }
};

} //provd
} //tcap
} //eyeline

#endif /* __ELC_TCAP_INDICATIONS_COMPOSERS_HPP */

