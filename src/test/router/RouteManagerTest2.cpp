#include "smpp/smpp.h"
#include "sms/sms.h"
#include "util/debug.h"
#include "smeman/smeman.h"
#include "router/route_manager.h"
#include "test/util/Util.hpp"

using namespace smsc::smpp;
using namespace smsc::sms;
using namespace smsc::smeman;
using namespace smsc::router;
using namespace smsc::test::util;

/*
using namespace smsc::test::util;
using namespace smsc::test::smeman; //constants, SmeManagerTestCases
using namespace smsc::test::router; //constants, TestRouteData
using log4cpp::Category;
using namespace smsc::sms::Address;
using smsc::util::Logger;
using smsc::smeman::SmeInfo;
using smsc::smeman::SmeProxy;
using smsc::test::sms::operator<<;
using smsc::test::sms::SmsUtil;
*/

const char* trim(char* str)
{
	int idx = 0;
	int len = strlen(str);
	for (; str[idx] == ' ' && idx < len; idx++) {}
	if (idx)
	{
		char tmp[len + 1];
		strncpy(tmp, str, len);
		strncpy(str, tmp + idx, len - idx);
		str[len - idx] = 0;
	}
}

void executeTest()
{
	RouteManager routeMan;
	SmeManager smeMan;
	routeMan.assign(&smeMan);
	/* */
	const char* fileName = "0.log";
	FILE* f = fopen(fileName, "r");
	if (f == NULL)
	{
		printf("Failed to open file %s", fileName);
		return;
	}
	char tmp[128];
	char srcVal[30];
	char dstVal[30];
	int srcNpi, srcTon, dstNpi, dstTon;
	while (fscanf(f, "@@@={SRC::%[^(]%[^,],n:%d,t:%d}{DST::%[^(]%[^,],n:%d,t:%d}%s\n",
		srcVal, tmp, &srcNpi, &srcTon, dstVal, tmp, &dstNpi, &dstTon, tmp) != EOF)
	{
		Address src, dst;
		src.setTypeOfNumber(srcTon);
		src.setNumberingPlan(srcNpi);
		trim(srcVal);
		src.setValue(strlen(srcVal), srcVal);
		dst.setTypeOfNumber(dstTon);
		dst.setNumberingPlan(dstNpi);
		trim(dstVal);
		dst.setValue(strlen(dstVal), dstVal);
		__trace2__("src=%s, dst=%s", srcVal, dstVal);
		//sme
		SmeInfo sme;
		sme.typeOfNumber = 0;
		sme.numberingPlan = 0;
		sme.interfaceVersion = 0x34;
		sme.rangeOfAddress = "";
		sme.systemType = "";
		sme.password = "";
		sme.hostname = "";
		sme.port = 0;
		auto_ptr<char> tmp = rand_char(15);
		sme.systemId = tmp.get();
		smeMan.addSme(sme);
		//route
		RouteInfo r;
		r.smeSystemId = sme.systemId;
		r.source = src;
		r.dest = dst;
		routeMan.addRoute(r);
	}
	fclose(f);
	routeMan.commit();
	//lookup
	Address src, dst;
	src.setTypeOfNumber(225);
	src.setNumberingPlan(170);
	const char* srcVal2 = "BHYULTFQSA";
	src.setValue(strlen(srcVal2), srcVal2);
	dst.setTypeOfNumber(138);
	dst.setNumberingPlan(50);
	const char* dstVal2 = "AQXUCALPCLPODE";
	dst.setValue(strlen(dstVal2), dstVal2);
	SmeProxy* proxy;
	bool found = routeMan.lookup(src, dst, proxy);
	__trace2__("Test: found = %d", (int) found);
}

int main(int argc, char* argv[])
{
	try
	{
		executeTest();
	}
	catch (...)
	{
		printf("Failed to execute test. See the logs\n");
	}

	return 0;
}

