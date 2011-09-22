/* ************************************************************************** *
 * Abonent Provider queries facility: aggregates registry of queries FSMs 
 * and pool of threads handling FSM switching events.
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAP_QUERY_FACILITY_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_IAP_QUERY_FACILITY_HPP

#include "core/buffers/DAHashT.hpp"
#include "core/synchronization/TimeSlice.hpp"

#include "inman/abprov/facility2/IAPQueryHFunc.hpp"
#include "inman/abprov/facility2/IAPNotifier.hpp"

namespace smsc {
namespace inman {
namespace iaprvd { //(I)NMan (A)bonent (P)roviders

using smsc::core::synchronization::TimeSlice;

class IAPQueryFacility : public IAPQueryProcessorITF
                       , protected IAPQueryRefereeIface {
public:
  enum State_e {
    fctIdle = 0
  , fctInited
  , fctStopping //no new workers allocated, only existing ones are served for a while
  , fctRunning
  };

  static const unsigned   _MAX_IDENT_SZ = 32;
  static const TimeSlice  _dflt_ShutdownTmo;  //default shutdown timeout = 300 millisecs.
                                              //Upon its expiration all active queries are killed.

  explicit IAPQueryFacility(const char * use_ident, Logger * use_log = NULL)
    : mState(fctIdle), mShtdTmo(&_dflt_ShutdownTmo), mQryPool(0), mlogId(use_ident)
    , mLogger(use_log ? use_log : Logger::getInstance(IAPROVIDER_DFLT_LOGGER))
    , mIapNtfr(use_ident, mLogger)
  { }
  //
  virtual ~IAPQueryFacility()
  {
    stop(true);
  }

  //Returns false in case of inconsistent facility state.
  //NOTE: setting max_threads to 0 turns off threads number limitation!
  bool init(IAPQueriesPoolIface & use_pool, uint16_t max_threads = 0);
  //
  void setShutdownTmo(const TimeSlice & use_tmo) { mShtdTmo = &use_tmo; }
  //Returns false in case of inconsistent facility state
  bool start(void);
  //Stops processing of a new queries (switches facility to fctStopping state).
  //if a 'do_wait' argument is set, kills all active queries as shutdown timeout
  //is expired and switches facility to fctInited state.
  State_e stop(bool do_wait);

  // -------------------------------------------------------
  // -- IAPQueryProcessorITF interface methods
  // -------------------------------------------------------
  //Starts query and binds listener to it.
  //Returns: true if query succesfully started, false otherwise
  virtual bool startQuery(const AbonentId & ab_number, IAPQueryListenerITF & pf_cb);
  //Unbinds query listener, cancels query if no listeners remain.
  //Returns: false if listener is already targeted and query waits for its mutex.
  virtual bool cancelQuery(const AbonentId & ab_number, IAPQueryListenerITF & pf_cb);
  //Attempts to cancel current queries, and switches facility to fctStopping state.
  //Returns: false if at least one query cann't be cancelled immediately
  //(i.e. waits for listener mutex).
  virtual bool cancelAllQueries(void);

protected:
  // -------------------------------------------------------
  // -- IAPQueryRefereeIface interface methods
  // -------------------------------------------------------
  virtual void onQueryEvent(IAPQueryId qry_id) /*throw()*/;

private:
  typedef smsc::core::buffers::DAHash_T <
    AbonentId, IAPQueryRef, smsc::core::buffers::DAHashSlot_T
  > WorkersMap;
  /* */
  mutable smsc::core::synchronization::Mutex  mSync;
  volatile State_e      mState;
  WorkersMap            mQryReg; //registry of monitored queries

  const TimeSlice *     mShtdTmo; //shutdown timeout
  IAPQueriesPoolIface * mQryPool; //pool of queries objects
  const char *          mlogId;   //prefix for logging info
  Logger *              mLogger;
  IAPNotifier           mIapNtfr;

  //Returns true if a query for given abonendId is registered.
  bool isRegistered(const AbonentId & ab_number) const;
  //Returns true if given query is registered.
  bool isRegistered(const IAPQueryRef & p_qry) const;
  //Attmpts to start a new query.
  bool activateQuery(const IAPQueryRef & p_qry, IAPQueryListenerITF & pf_cb);
  //NOTE: query MUST be locked upon entry!!!
  void procQueryEvent(const IAPQueryRef & p_qry);
};

} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAP_QUERY_FACILITY_HPP */

