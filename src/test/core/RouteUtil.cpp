#include "RouteUtil.hpp"

namespace smsc {
namespace test {
namespace core {

using smsc::sms::AddressValue;

ostream& operator<< (ostream& os, const RouteInfo& route)
{
	int len;
	AddressValue tmp;
	os << "routeId = " << route.routeId;
	os << ", smeSystemId = " << route.smeSystemId << "(" <<
		route.smeSystemId.length() << ")";
	len = route.source.getValue(tmp);
	os << ", source = " << tmp << "(" << len << ")";
	len = route.dest.getValue(tmp);
	os << ", dest = " << tmp << "(" << len << ")";
	os << ", priority = " << route.priority;
	os << ", billing = " << (route.billing ? "true" : "false");
	os << ", paid = " << (route.paid ? "true" : "false");
	os << ", archived = " << (route.archived ? "true" : "false");
	return os;
}

ostream& operator<< (ostream& os, const TestRouteData& data)
{
	int len;
	AddressValue tmp;
	os << "match = " << (data.match ? "true" : "false");
	len = data.origAddr.getValue(tmp);
	os << ", origAddr = " << tmp << "(" << len << ")";
	len = data.destAddr.getValue(tmp);
	os << ", destAddr = " << tmp << "(" << len << ")";
	os << ", origAddrMatch = " << data.origAddrMatch;
	os << ", destAddrMatch = " << data.destAddrMatch;
	os << ", route = {" << *data.route << "}";
	return os;
}

}
}
}

