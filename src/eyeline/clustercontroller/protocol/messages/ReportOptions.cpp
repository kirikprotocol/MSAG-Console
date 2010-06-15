#include "ReportOptions.hpp"

namespace eyeline{
namespace clustercontroller{
namespace protocol{
namespace messages{

const int8_t ReportOptions::ReportNone=0;
const int8_t ReportOptions::ReportFull=1;
const int8_t ReportOptions::ReportFinal=2;
std::map<ReportOptions::type,std::string> ReportOptions::nameByValue;
std::map<std::string,ReportOptions::type> ReportOptions::valueByName;
ReportOptions::StaticInitializer ReportOptions::staticInitializer;

}
}
}
}
