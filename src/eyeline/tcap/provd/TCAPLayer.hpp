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

  // ----------------------------------
  // -- TCAPIface interface methods
  // ----------------------------------
  virtual TDialogueId openDialogue(TDlgHandlerIface* tIndicationHandlers,
                                   unsigned int dialogueTimeout);
  virtual void updateDialogue(const TC_Begin_Req & req_primitive);
  virtual void updateDialogue(const TC_Cont_Req & req_primitive);
  virtual void updateDialogue(const TC_End_Req & req_primitive);
  virtual void updateDialogue(const TC_UAbort_Req & req_primitive);
  virtual void closeDialogue(const TDialogueId& tDialogueId);
  // ----------------------------------
  void updateDialogue(const TC_PAbort_Req & req_primitive);

  void sendPrimitive(const TPAbortReqComposer & req_composer, unsigned int link_num,
                     const SCCPAddress& src_addr, const SCCPAddress& dst_addr) const;
  void sendPrimitive(const TUAbortReqComposer & req_composer, unsigned int link_num,
                     const SCCPAddress& src_addr, const SCCPAddress& dst_addr) const;

private:
  smsc::logger::Logger* _logger;
  OutPrimitivesProcessor* _outPrimitivesProcessor;
  MsgReadersPool* _msgReaders;

  bool _wasInitialized;
  unsigned _maxDialoguesCount, _dialogueTimeout, _rosTimeout;

  sua::libsua::SuaApi* _suaApi;
};

}}}

#endif
