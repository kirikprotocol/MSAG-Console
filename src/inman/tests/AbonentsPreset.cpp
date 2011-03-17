#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */
/* ************************************************************************** *
 * 
 * ************************************************************************** */

#include "inman/tests/AbonentsPreset.hpp"

namespace smsc  {
namespace inman {
namespace test {

const AbonentPreset  _knownAbonents[] = {
  //Nezhinsky phone(prepaid):
    AbonentPreset(AbonentContractInfo::abtPrepaid, ".1.1.79139343290", "250013900405871")
  //tst phone (prepaid):
  , AbonentPreset(AbonentContractInfo::abtPrepaid, ".1.1.79133821781", "250013903368782")
  //Ryzhkov phone(postpaid):
  , AbonentPreset(AbonentContractInfo::abtPrepaid, ".1.1.79139859489", "250013901464251")
  //Stupnik phone(postpaid):
  , AbonentPreset(AbonentContractInfo::abtPrepaid, ".1.1.79139033669", "250013901464251")
  //ATSI integration: test abonents
  , AbonentPreset(AbonentContractInfo::abtPrepaid,  ".1.1.79104075344", "250016170023531")
  , AbonentPreset(AbonentContractInfo::abtPostpaid, ".1.1.79169300003", "250016916495004")
  , AbonentPreset(AbonentContractInfo::abtPostpaid,  ".1.1.79165603331", "250016310029598")
  , AbonentPreset(AbonentContractInfo::abtUnknown,  ".1.1.79166699601", 0)
  , AbonentPreset(AbonentContractInfo::abtPostpaid,  ".1.1.79857603330", "250016800256834")
  , AbonentPreset(AbonentContractInfo::abtPostpaid,  ".1.1.79166803330", "250016800074961")
  , AbonentPreset(AbonentContractInfo::abtPostpaid,  ".1.1.79857860099", "250016903052714")
  , AbonentPreset(AbonentContractInfo::abtPrepaid,  ".1.1.79161690056", "250016909174125")
  , AbonentPreset(AbonentContractInfo::abtPrepaid,  ".1.1.79166199999", "250016005034406")
  , AbonentPreset(AbonentContractInfo::abtPrepaid,  ".1.1.79162624625", "250016258257746")
  , AbonentPreset(AbonentContractInfo::abtPrepaid,  ".1.1.79167595907", "250016003219998")
  , AbonentPreset(AbonentContractInfo::abtPrepaid,  ".1.1.79160305277", "250016170023769")
  , AbonentPreset(AbonentContractInfo::abtPrepaid,  ".1.1.79853646295", "250016170023531")
  , AbonentPreset(AbonentContractInfo::abtPostpaid,  ".1.1.79163841468", "250016907163177")
  , AbonentPreset(AbonentContractInfo::abtPostpaid,  ".1.1.79169037692", "250016918392913")
  , AbonentPreset(AbonentContractInfo::abtPostpaid,  ".1.1.79168056148", "250016819159666")
  , AbonentPreset(AbonentContractInfo::abtPrepaid,  ".1.1.79104081922", "250016170023652")
  , AbonentPreset(AbonentContractInfo::abtPrepaid,  ".1.1.79104082370", "250016170023659")
  , AbonentPreset(AbonentContractInfo::abtPostpaid,  ".1.1.79154831016", "250015480000150")
};
const unsigned _knownAbonentsNum = (sizeof(_knownAbonents)/sizeof(AbonentPreset));

} //test
} //inman
} //smsc

