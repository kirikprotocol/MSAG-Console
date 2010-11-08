#include "ReqField.hpp"

namespace eyeline{
namespace informer{
namespace dcp{
namespace messages{

const int8_t ReqField::State;
const int8_t ReqField::Date;
const int8_t ReqField::Abonent;
const int8_t ReqField::Text;
const int8_t ReqField::ErrorCode;
const int8_t ReqField::UserData;
std::map<ReqField::type,std::string> ReqField::nameByValue;
std::map<std::string,ReqField::type> ReqField::valueByName;
ReqField::StaticInitializer ReqField::staticInitializer;

}
}
}
}
