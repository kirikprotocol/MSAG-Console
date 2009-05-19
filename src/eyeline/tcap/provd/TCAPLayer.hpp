#ifndef __EYELINE_TCAP_PROVD_TCAPLAYER_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_TCAPLAYER_HPP__

# include "util/config/ConfigView.h"
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/sua/libsua/SuaUser.hpp"

# include "eyeline/tcap/TCAPIface.hpp"
# include "eyeline/tcap/provd/OutPrimitivesProcessor.hpp"
# include "eyeline/tcap/provd/MsgReadersPool.hpp"
# include "eyeline/tcap/TDlgHandlerIfaceFactory.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

class TCAPLayer : public utilx::Singleton<TCAPLayer>,
                  public TCAPIface {
public:
  TCAPLayer();
  ~TCAPLayer();

  void initialize(smsc::util::config::ConfigView& tc_provider_cfg_section);
  void shutdown();

  TCAPIface* getTCAPIface();

  void registerAppCTX(const EncodedOID& ctx,
                      TDlgHandlerIfaceFactory* indHndlrsFactory,
                      unsigned int dialogueTimeout);

  virtual TDialogueId openDialogue(TDlgHandlerIface* tIndicationHandlers,
                                   unsigned int dialogueTimeout);

  virtual void updateDialogue(TC_Begin_Req* requestPrimitive);
  virtual void updateDialogue(TC_Cont_Req* requestPrimitive);
  virtual void updateDialogue(TC_End_Req* requestPrimitive);
  virtual void updateDialogue(TC_UAbort_Req* requestPrimitive);

  virtual void closeDialogue(const TDialogueId& tDialogueId);

  void updateDialogue(TC_PAbort_Req* requestPrimitive);

  void sendPrimitive(TC_PAbort_Req* requestPrimitive, unsigned int linkNum,
                     const SCCPAddress& src_addr, const SCCPAddress& dst_addr);
  void sendPrimitive(TC_UAbort_Req* requestPrimitive, unsigned int linkNum,
                     const SCCPAddress& src_addr, const SCCPAddress& dst_addr);

private:
  OutPrimitivesProcessor* _outPrimitivesProcessor;
  MsgReadersPool* _msgReaders;

  bool _wasInitialized;
  unsigned _maxDialoguesCount, _dialogueTimeout, _rosTimeout;

  sua::libsua::SuaApi* _suaApi;
};

}}}

#endif
