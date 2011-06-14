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

using smsc::inman::interaction::INPAbntContract;
using smsc::inman::interaction::AbntContractRequest;
using smsc::inman::interaction::AbntContractResult;

/* ************************************************************************** *
 * class DtcrDialog: INMan AbonentContract detection dialog params and result
 * ************************************************************************** */

class DtcrFacade; //forward declaration

class DtcrDialog {
public:
  enum DlgState { dIdle = 0, dRequested, dReported };

protected:
  DtcrFacade *    _mgr;
  unsigned        dId;
  unsigned        abId;   //abonent's id from AbonentsDB
  AbonentInfo *   abInfo;
  DlgState        state;
  bool            useCache;

public:
  DtcrDialog(DtcrFacade * use_mgr, unsigned dlg_id, unsigned ab_id, bool use_cache = true);
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
class DtcrFacade : public TSTFacadeAC, smsc::inman::interaction::AbntContractResHandlerITF {
protected:
  typedef std::map<unsigned, DtcrDialog*> DtcrDialogsMap;

  AbonentsDB &        _abDB;
  unsigned            _maxDlgId;
  DtcrDialogsMap      _Dialogs;

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
  static const INPAbntContract  _protoDef; //provided protocol definition

  DtcrFacade(AbonentsDB & use_db, TcpServerIface & conn_srv, Logger * use_log = NULL);
  virtual ~DtcrFacade();

  AbonentsDB & getAbntDb(void) { return _abDB; }


  bool detectAbn(unsigned ab_id, bool use_cache = true);
  bool detectAbn(const TonNpiAddress & sbscr, bool use_cache = true);

  //detects contract for number of abonents from DB starting from given abId
  unsigned detectAbnMlt(unsigned ab_id, unsigned range, bool use_cache = true);
  //detects contract for number of abonents starting from given ISDN address
  unsigned detectAbnMlt(const TonNpiAddress & sbscr, unsigned range, bool use_cache = true);

  // ------------------------------------------------------------
  // -- PacketListenerIface interface methods:
  // ------------------------------------------------------------
  //Returns true if listener has utilized packet so no more listeners
  //should be notified, false - otherwise (in that case packet will be
  //reported to other listeners).
  virtual bool onPacketReceived(unsigned conn_id, PacketBufferAC & recv_pck)
    /*throw(std::exception) */;
};

} // namespace test
} // namespace inman
} // namespace smsc
#endif /* __SMSC_INMAN_TEST_ABONENT_DETECTOR_HPP */

