/* ************************************************************************** *
 * Helper classes for operations with RPCauses (Reject Processing Cause)
 * ************************************************************************** */
#ifndef __SMSC_INMAN_RPCLIST__
#ident "@(#)$Id$"
#define __SMSC_INMAN_RPCLIST__

#include "util/csv/CSVListOf.hpp"
#include "util/csv/CSValueTraitsInt.hpp"

namespace smsc   {
namespace inman  {
namespace common {

//Ordinary list of RPCauses, format: "C1, C2, ..., Cn"
class RPCList : public util::csv::CSVListOf_T<uint8_t> {
public:
  RPCList() : util::csv::CSVListOf_T<uint8_t>()
  { }
  ~RPCList()
  { }

  bool exist(uint8_t rp_cause) const
  {
    for (RPCList::const_iterator it = RPCList::begin(); it != RPCList::end(); ++it) {
      if (*it == rp_cause)
        return true;
    }
    return false;
  }
};

//RPCause with unsigned int attribute
struct RPCauseATT {
  uint8_t   _rpc;
  uint16_t  _att;

  RPCauseATT(uint8_t use_cause = 0, uint16_t use_att = 0)
    : _rpc(use_cause), _att(use_att)
  { }

  static RPCauseATT   str2val(const std::string & str_val) /*throw(std::exception)*/;
  static std::string  val2str(const RPCauseATT & use_val) /*throw()*/;
};

} //common
} //inman
} //smsc


namespace smsc {
namespace util {
namespace csv {

template </* _TArg = RPCauseATT */> 
struct CSValueTraits_T<inman::common::RPCauseATT> {
  static inman::common::RPCauseATT str2val(const std::string & str_val) /*throw(std::exception)*/
  {
    return inman::common::RPCauseATT::str2val(str_val);
  }
  static std::string val2str(const inman::common::RPCauseATT & use_val) /*throw()*/
  {
    return inman::common::RPCauseATT::val2str(use_val);
  }
};

} //csv
} //util
} //smsc


namespace smsc   {
namespace inman  {
namespace common {

//List of RPCauses with attributes,
//Format: "C1:A1, C2:A2, ..., Cn:An"
class RPCListATT : public util::csv::CSVListOf_T<RPCauseATT> {
public:
  RPCListATT() : util::csv::CSVListOf_T<RPCauseATT>()
  { }
  ~RPCListATT()
  { }

  const RPCauseATT * exist(uint8_t rp_cause) const
  {
    for (RPCListATT::const_iterator it = RPCListATT::begin();
                                    it != RPCListATT::end(); ++it) {
      if (it->_rpc == rp_cause)
        return it.operator->();
    }
    return NULL;
  }
};

} //common
} //inman
} //smsc

#endif /* __SMSC_INMAN_RPCLIST__ */

