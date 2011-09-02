/* ************************************************************************** *
 * SMS/USSD Billing service config file parsing.
 * ************************************************************************** */
#ifndef __INMAN_BILLMODES_CONFIG_PARSING
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_BILLMODES_CONFIG_PARSING

#include "inman/services/ICSCfgReader.hpp"
#include "inman/services/common/BillingModes.hpp"

namespace smsc   {
namespace inman  {

using smsc::logger::Logger;
using smsc::util::config::Config;
using smsc::util::config::ConfigException;


class BillModesCfgReader {
private:
  Logger *        mLogger;
  const char *    mNmSec;

public:
  explicit BillModesCfgReader(Logger * use_log, const char * nm_sec = "BillingModes")
    : mLogger(use_log), mNmSec(nm_sec)
  { }
  ~BillModesCfgReader()
  { }

  const char * nmCfgSection(void) const { return mNmSec; }

  //NOTE: 'root_sec' is a Config associated with a file containing BillingModes section
  void parseSection(const Config & root_sec, const char * nm_sub_sec,
                    TrafficBillModes & st_cfg) const throw(ConfigException);

protected:
  /* Reads BillingModes subsection */
  void readBillingModes(const smsc::util::config::XConfigView & sub_cfg,
                        TrafficBillModes & st_cfg) const throw(ConfigException);
  //
  void readModesFor(MSG_TYPE msg_type, const smsc::util::config::XConfigView & msg_cfg,
                    TrafficBillModes & st_cfg) const throw(ConfigException);
  //
  void readBillMode(MSG_TYPE msg_type, const char * mode_str,
                    BillModes & bm_cfg) const throw(ConfigException);
  //
  static void str2BillMode(const char * mode_str, BILL_MODE (& bm_prio)[2]) throw(ConfigException);
};


} //inman
} //smsc
#endif /* __INMAN_BILLMODES_CONFIG_PARSING */

