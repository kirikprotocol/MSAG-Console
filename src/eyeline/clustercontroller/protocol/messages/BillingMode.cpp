#include "BillingMode.hpp"

namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{

const int8_t BillingMode::BillOff=0;
const int8_t BillingMode::BillOn=1;
const int8_t BillingMode::BillMt=2;
const int8_t BillingMode::BillFr=3;
const int8_t BillingMode::BillOnSubmit=4;
const int8_t BillingMode::BillCdr=5;
std::map<BillingMode::type,std::string> BillingMode::nameByValue;
std::map<std::string,BillingMode::type> BillingMode::valueByName;
BillingMode::StaticInitializer BillingMode::staticInitializer;

}
}
}
}
