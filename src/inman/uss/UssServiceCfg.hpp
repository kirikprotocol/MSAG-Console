#ifndef __SMSC_INMAN_USS_USSSERVICE_CFG_HPP__
# define __SMSC_INMAN_USS_USSSERVICE_CFG_HPP__ 1

# include <inman/interaction/server.hpp>
# include <inman/inman.hpp>

namespace smsc  {
namespace inman {
namespace uss  {

struct SS7_CFG {    //SS7 stack interaction:
  unsigned char   userId;         //PortSS7 user id [1..20]
  unsigned short  capTimeout;     //optional timeout for operations with IN platform
  TonNpiAddress   ssf_addr;       //local ISDN address
  int             own_ssn;        //local SSN
  unsigned short  maxDlgId;       //maximum number of simultaneous TC dialogs, max: 65530
  unsigned char   fake_ssn;
};

struct UssService_CFG {
  smsc::inman::interaction::ServSocketCFG sock;
  SS7_CFG                                 ss7;            //SS7 interaction:
};

}
}
}

#endif
