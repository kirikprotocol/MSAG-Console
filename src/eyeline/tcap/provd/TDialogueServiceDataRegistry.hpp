#ifndef __EYELINE_TCAP_PROVD_TDIALOGUESERVICEDATAREGISTRY_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_TDIALOGUESERVICEDATAREGISTRY_HPP__

# include <map>

# include "core/buffers/RefPtr.hpp"
# include "core/synchronization/Mutex.hpp"
# include "util/Exception.hpp"

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/tcap/TDialogueId.hpp"
# include "eyeline/tcap/TDlgHandlerIface.hpp"
# include "eyeline/tcap/proto/TransactionId.hpp"
# include "eyeline/tcap/provd/TDialogueServiceData.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

class TDialogueServiceDataRegistry : public utilx::Singleton<TDialogueServiceDataRegistry> {
public:
  TDialogueServiceDataRegistry();

  typedef smsc::core::buffers::RefPtr<TDialogueServiceData, smsc::core::synchronization::Mutex> registry_element_ref_t;

  registry_element_ref_t createTDialogueServiceData(TDlgHandlerIface* tIndicationHandlers,
                                                    unsigned int dialogueTimeout);
  registry_element_ref_t createTDialogueServiceData(TDlgHandlerIface* tIndicationHandlers,
                                                    const proto::TransactionId& remoteTrnId,
                                                    unsigned int dialogueTimeout);

  registry_element_ref_t getTDialogueServiceData(const TDialogueId& dialogueId);
  registry_element_ref_t getTDialogueServiceData(const proto::TransactionId& transactionId);

  void destroyTDialogueServiceData(const TDialogueId& dialogueId);

private:
  void setSearchKeys(const proto::TransactionId& trnId,
                     const TDialogueId& dialogueId,
                     registry_element_ref_t& tDlgSvcData);

  void allocateDialogueIdAndLocalTransactionId(uint32_t* localTrnId,
                                               TDialogueId* dialogueId);

  smsc::core::synchronization::Mutex _lockForIdGeneration, _lockForDataStructures;
  uint32_t _localTrnId, _dialogueId;

  typedef std::map<proto::TransactionId, registry_element_ref_t> trnId_2_dlgInfo_t;
  trnId_2_dlgInfo_t _trnIdTo2DlgInfo;

  typedef std::map<TDialogueId, registry_element_ref_t> dlgId_2_dlgInfo_t;
  dlgId_2_dlgInfo_t _dlgId2DlgInfo;
};

}}}

#endif
