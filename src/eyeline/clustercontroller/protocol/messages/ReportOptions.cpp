#include "ReportOptions.hpp"

namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{

const int8_t ReportOptions::ReportNone;
const int8_t ReportOptions::ReportFull;
const int8_t ReportOptions::ReportFinal;
std::map<ReportOptions::type,std::string> ReportOptions::nameByValue;
std::map<std::string,ReportOptions::type> ReportOptions::valueByName;
ReportOptions::StaticInitializer ReportOptions::staticInitializer;

}
}
}
}
