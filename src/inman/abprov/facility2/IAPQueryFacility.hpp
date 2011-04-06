/* ************************************************************************** *
 * Abonent Provider queries facility: aggregates registry of queries FSMs 
 * and pool of threads handling FSM switching events.
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAP_QUERY_FACILITY_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_INMAN_IAP_QUERY_FACILITY_HPP

#include "core/synchronization/TimeSlice.hpp"
#include "inman/abprov/facility2/IAPNotifier.hpp"


namespace smsc {
namespace inman {
namespace iaprvd { //(I)NMan (A)bonent (P)roviders

using smsc::core::synchronization::TimeSlice;

class IAPQueryFacility : public IAPQueryProcessorITF {
private:
  const char *        _logId;     //prefix for logging info
  IAPQueriesStore     _qrsStore;
  IAPNotifier         _iapNtfr;
  Logger *            _logger;

public:
  static const unsigned   _MAX_IDENT_SZ = 32;
  static const TimeSlice  _dflt_ShutdownTmo; //300 millisecs

  explicit IAPQueryFacility(const char * use_ident, Logger * use_log = NULL)
    : _logId(use_ident), _iapNtfr(use_ident, _qrsStore, use_log), _logger(use_log)
  {
    if (!_logger)
      _logger = Logger::getInstance(IAPROVIDER_DFLT_LOGGER);
  }
  //
  virtual ~IAPQueryFacility()
  {
    cancelAllQueries();
  }

  void init(IAPQueriesPoolIface & use_pool, uint16_t max_threads)
  {
    _qrsStore.init(use_pool, _iapNtfr);
    _iapNtfr.init(1, max_threads);
  }
  //
  void start(void)  { _iapNtfr.start(); }
  //
  void stopNotify() { _iapNtfr.stopNotify(); }
  //Finally stops facility. Cancels all existing queries.
  //If shutdown timeout isn't set the default one is used.
  void stop(const TimeSlice * use_tmo = NULL);

  // -------------------------------------------------------
  // -- IAPQueryProcessorITF interface methods
  // -------------------------------------------------------
  //Starts query and binds listener to it.
  //Returns: true if query succesfully started, false otherwise
  virtual bool startQuery(const AbonentId & ab_number, IAPQueryListenerITF & pf_cb);
  //Unbinds query listener, cancels query if no listeners remain.
  //Returns: false if listener is already targeted and query waits for its mutex.
  virtual bool cancelQuery(const AbonentId & ab_number, IAPQueryListenerITF & pf_cb);
  //Returns: false if at least one query cann't be cancelled immediately
  //(i.e. waits for listener mutex).
  virtual bool cancelAllQueries(void);
};

} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAP_QUERY_FACILITY_HPP */

