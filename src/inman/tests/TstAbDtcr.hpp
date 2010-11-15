/* ************************************************************************** *
 * INMan Contract Determination protocol client
 * ************************************************************************** */
#ifndef __SMSC_INMAN_TEST_ABONENT_DETECTOR_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_TEST_ABONENT_DETECTOR_HPP

#include "inman/tests/AbonentsDB.hpp"
#include "inman/tests/TstFacade.hpp"

#include "inman/interaction/msgdtcr/MsgContract.hpp"

namespace smsc  {
namespace inman {
namespace test {

using smsc::inman::test::TSTFacadeAC;
using smsc::inman::test::AbonentsDB;

using smsc::inman::interaction::AbntContractRequest;
using smsc::inman::interaction::AbntContractResult;
using smsc::inman::interaction::AbntContractResHandlerITF;


/* ************************************************************************** *
 * class DtcrDialog: INMan AbonentContract detection dialog params and result
 * ************************************************************************** */
class DtcrDialog {
public:
  enum DlgState { dIdle = 0, dRequested, dReported };

protected:
  TSTFacadeAC *   _mgr;
  unsigned        dId;
  unsigned        abId;   //abonent's id from AbonentsDB
  AbonentInfo *   abInfo;
  DlgState        state;
  bool            useCache;

public:
  DtcrDialog(TSTFacadeAC * use_mgr, unsigned dlg_id, unsigned ab_id, bool use_cache = true)
    : _mgr(use_mgr), dId(dlg_id), abId(ab_id), state(dIdle), useCache(use_cache)
  { 
    abInfo = AbonentsDB::getInstance()->getAbnInfo(ab_id);
  }
  ~DtcrDialog()
  { }

  unsigned getId(void) const { return dId; }
  void     setState(DlgState new_state) { state = new_state; }
  DlgState getState(void) const { return state; }
  AbonentInfo * abnInfo(void) const { return abInfo; }

  //Composes and sends request, num_bytes == 0 forces sending whole packet
  bool sendRequest(unsigned num_bytes = 0);
  //
  void onResult(AbntContractResult * res);
};

/* ************************************************************************** *
 * class DtcrFacade: INMan AbonentContract detection dialogs controlling registry
 * ************************************************************************** */
class DtcrFacade : public TSTFacadeAC, AbntContractResHandlerITF {
protected:
  typedef std::map<unsigned, DtcrDialog*> DtcrDialogsMap;

  unsigned            _maxDlgId;
  DtcrDialogsMap      _Dialogs;
  AbonentsDB *        _abDB;

  unsigned      nextDialogId(void) { return ++_maxDlgId; }
  DtcrDialog *  findDialog(unsigned did) const;
  DtcrDialog *  initDialog(unsigned ab_id, bool use_cache = true);
  void          eraseDialog(unsigned did);

  bool numericIncrement(char * buf, unsigned len);

  // ---------------------------------------------------
  // -- AbntContractResHandlerITF interface implementation
  // ---------------------------------------------------
  void onContractResult(AbntContractResult * res, uint32_t req_id);

public:
  explicit DtcrFacade(ConnectSrv * conn_srv, Logger * use_log = NULL);
  virtual ~DtcrFacade();

  bool detectAbn(unsigned ab_id, bool use_cache = true);
  bool detectAbn(const TonNpiAddress & sbscr, bool use_cache = true);

  //detects contract for number of abonents from DB starting from given abId
  unsigned detectAbnMlt(unsigned ab_id, unsigned range, bool use_cache = true);
  //detects contract for number of abonents starting from given ISDN address
  unsigned detectAbnMlt(const TonNpiAddress & sbscr, unsigned range, bool use_cache = true);

  // ---------------------------------------------------
  // -- ConnectListenerITF interface implementation
  // ---------------------------------------------------
  virtual void onPacketReceived(Connect * conn, std::auto_ptr<SerializablePacketAC> & recv_cmd)
    /*throw(std::exception) */;
};

} // namespace test
} // namespace inman
} // namespace smsc
#endif /* __SMSC_INMAN_TEST_ABONENT_DETECTOR_HPP */

