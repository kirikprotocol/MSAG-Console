#include "SMUtil.hpp"
#include <cstring>

namespace smsc  {
namespace test  {
namespace store {

using namespace std;
using namespace smsc::sms;

bool SMUtil::compareAddresses(Address& a1, Address& a2)
{
	bool res = &a1 != NULL && &a2 != NULL &&
		a1.getLenght() == a2.getLenght() &&
		a1.getTypeOfNumber() == a2.getTypeOfNumber() &&
		a1.getNumberingPlan() == a2.getNumberingPlan();
	if (res)
	{
		res &= memcmp(a1.value, a2.value, a1.getLenght()) == 0;
	}
	return res;
}
	
bool SMUtil::compareMessageBody(Body& b1, Body& b2)
{
	bool res = &b1 != NULL && &b2 != NULL &&
		b1.isHeaderIndicator() == b2.isHeaderIndicator() &&
		b1.getCodingScheme() == b2.getCodingScheme() &&
		b1.lenght == b2.lenght;
	if (res)
	{
		res &= memcmp(b1.data, b2.data, b1.lenght) == 0;
	}
	return res;
}

}
}
}

