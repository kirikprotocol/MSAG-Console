#include "ConfigUtil.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/smeman/SmeManagerTestCases.hpp"
#include "test/core/RouteUtil.hpp"
#include "test/util/Util.hpp"
#include "smeman/smeinfo.h"
#include "alias/aliasman.h"

namespace smsc {
namespace test {
namespace config {

using std::string;
using smsc::smeman::SmeInfo;
using smsc::alias::AliasInfo;
using smsc::router::RouteInfo;
using smsc::test::conf::TestConfig;
using smsc::test::smeman::SmeManagerTestCases;
using smsc::test::sms::operator==;
using namespace smsc::test::sms;
using namespace smsc::test::core;
using namespace smsc::test::util;

void ConfigUtil::setupSystemSme()
{
	//smsc sme
	__cfg_addr__(smscAddr);
	__cfg_addr__(smscAlias);
	__cfg_str__(smscSystemId);
	SmeInfo smscSme;
	smscSme.wantAlias = rand0(1);
	SmeManagerTestCases::setupRandomCorrectSmeInfo(&smscSme);
	smscSme.systemId = smscSystemId;
	smeReg->registerSme(smscAddr, smscSme, false, true);
	smeReg->bindSme(smscSme.systemId, SME_TRANSCEIVER);
	//алиас для smsc sme
	AliasInfo smscAliasInfo;
	smscAliasInfo.addr = smscAddr;
	smscAliasInfo.alias = smscAlias;
	smscAliasInfo.hide = true; //rand0(2);
	aliasReg->putAlias(smscAliasInfo);
	//регистрация profiler
	__cfg_addr__(profilerAddr);
	__cfg_addr__(profilerAlias);
	__cfg_str__(profilerSystemId);
	SmeInfo profilerSme;
	profilerSme.wantAlias = false;
	SmeManagerTestCases::setupRandomCorrectSmeInfo(&profilerSme);
	profilerSme.systemId = profilerSystemId;
	smeReg->registerSme(profilerAddr, profilerSme, false, true);
	smeReg->bindSme(profilerSme.systemId, SME_TRANSCEIVER);
	//алиас для profiler
	AliasInfo profilerAliasInfo;
	profilerAliasInfo.addr = profilerAddr;
	profilerAliasInfo.alias = profilerAlias;
	profilerAliasInfo.hide = true; //rand0(2);
	aliasReg->putAlias(profilerAliasInfo);
	//abonent info
	__cfg_addr__(abonentInfoAddrSme);
	__cfg_addr__(abonentInfoAddrMobile);
	__cfg_addr__(abonentInfoAliasSme);
	__cfg_addr__(abonentInfoAliasMobile);
	__cfg_str__(abonentInfoSystemId);
	SmeInfo abonentInfoSme;
	abonentInfoSme.wantAlias = false;
	SmeManagerTestCases::setupRandomCorrectSmeInfo(&abonentInfoSme);
	abonentInfoSme.systemId = abonentInfoSystemId;
	vector<const Address*> addrList;
	addrList.push_back(new Address(abonentInfoAddrSme));
	addrList.push_back(new Address(abonentInfoAddrMobile));
	smeReg->registerSme(addrList, abonentInfoSme, false, true);
	smeReg->bindSme(abonentInfoSme.systemId, SME_TRANSCEIVER);
	//алиас для abonent info
	AliasInfo abonentInfoSmppAliasInfo;
	abonentInfoSmppAliasInfo.addr = abonentInfoAddrSme;
	abonentInfoSmppAliasInfo.alias = abonentInfoAliasSme;
	abonentInfoSmppAliasInfo.hide = true; //rand0(2);
	aliasReg->putAlias(abonentInfoSmppAliasInfo);
	AliasInfo abonentInfoMapAliasInfo;
	abonentInfoMapAliasInfo.addr = abonentInfoAddrMobile;
	abonentInfoMapAliasInfo.alias = abonentInfoAliasMobile;
	abonentInfoMapAliasInfo.hide = true; //rand0(2);
	aliasReg->putAlias(abonentInfoMapAliasInfo);
	//distr list sme
	__cfg_addr__(distrListAddr);
	__cfg_addr__(distrListAlias);
	__cfg_str__(distrListSystemId);
	SmeInfo distrListSme;
	distrListSme.wantAlias = rand0(1);
	SmeManagerTestCases::setupRandomCorrectSmeInfo(&distrListSme);
	distrListSme.systemId = distrListSystemId;
	smeReg->registerSme(distrListAddr, distrListSme, false, true);
	smeReg->bindSme(distrListSme.systemId, SME_TRANSCEIVER);
	//алиас для distr list sme
	AliasInfo distrListAliasInfo;
	distrListAliasInfo.addr = distrListAddr;
	distrListAliasInfo.alias = distrListAlias;
	distrListAliasInfo.hide = true; //rand0(2);
	aliasReg->putAlias(distrListAliasInfo);
	//регистрация map proxy делается тестовой sme
}

void ConfigUtil::setupSystemSmeRoutes()
{
	vector<const Address*> addr;
	vector<const SmeSystemId*> smeId;
	//smsc sme
	__cfg_addr__(smscAddr);
	__cfg_addr__(smscAlias);
	__cfg_str__(smscSystemId);
	addr.push_back(&smscAddr);
	addr.push_back(&smscAlias);
	smeId.insert(smeId.end(), 2, &smscSystemId);
	//profiler
	__cfg_addr__(profilerAddr);
	__cfg_addr__(profilerAlias);
	__cfg_str__(profilerSystemId);
	addr.push_back(&profilerAddr);
	addr.push_back(&profilerAlias);
	smeId.insert(smeId.end(), 2, &profilerSystemId);
	//abonent info
	__cfg_addr__(abonentInfoAddrSme);
	__cfg_addr__(abonentInfoAddrMobile);
	__cfg_addr__(abonentInfoAliasSme);
	__cfg_addr__(abonentInfoAliasMobile);
	__cfg_str__(abonentInfoSystemId);
	addr.push_back(&abonentInfoAddrSme);
	addr.push_back(&abonentInfoAddrMobile);
	addr.push_back(&abonentInfoAliasSme);
	addr.push_back(&abonentInfoAliasMobile);
	smeId.insert(smeId.end(), 4, &abonentInfoSystemId);
	//map proxy
	__cfg_str__(mapProxySystemId);
	//create routes
	__require__(addr.size() == smeId.size());
	for (int i = 0; i < addr.size(); i++)
	{
		for (int j = 0; j < addr.size(); j++)
		{
			RouteInfo route;
			RouteUtil::setupRandomCorrectRouteInfo(&route);
			route.source = *addr[i];
			route.dest = *addr[j];
			route.smeSystemId = *smeId[j];
			route.enabling = true;
			routeReg->putRoute(route, NULL);
		}
	}
}

void ConfigUtil::setupRoute(const Address& srcAddr, const Address& destAddr,
	const SmeSystemId destSmeId)
{
	RouteInfo route;
	RouteUtil::setupRandomCorrectRouteInfo(&route);
	route.source = srcAddr;
	route.dest = destAddr;
	route.smeSystemId = destSmeId;
	route.enabling = true;
	routeReg->putRoute(route, NULL);
}

void ConfigUtil::setupDuplexRoutes(const Address& addr1, const SmeSystemId smeId1,
	const Address& addr2, const SmeSystemId smeId2)
{
	setupRoute(addr1, addr2, smeId2);
	setupRoute(addr2, addr1, smeId1);
}

pair<SmeType, SmeSystemId> ConfigUtil::checkRoute(const Address& origAddr,
	const SmeSystemId& origSmeId, const Address& destAlias)
{
	const Address destAddr = aliasReg->findAddressByAlias(destAlias);
	const RouteHolder* routeHolder = routeReg->lookup(origAddr, destAddr);
	if (routeHolder)
	{
		const SmeSystemId& smeId = routeHolder->route.smeSystemId;
		SmeType smeType = smeReg->getSmeBindType(smeId);
		__trace2__("route: origAddr = %s, origSmeId = %s, destAias = %s, route to = %s, sme type = %d",
			str(origAddr).c_str(), origSmeId.c_str(), str(destAlias).c_str(), smeId.c_str(), smeType);
		return make_pair(smeType, smeId);
	}
	__trace2__("route: origAddr = %s, origSmeId = %s, destAias = %s, no route",
		str(origAddr).c_str(), origSmeId.c_str(), str(destAlias).c_str());
	return make_pair(SME_NO_ROUTE, "");
}

void ConfigUtil::checkRoute2(const Address& origAddr, const SmeSystemId& origSmeId,
	const Address& destAlias)
{
	const Address destAddr = aliasReg->findAddressByAlias(destAlias);
	const RouteHolder* routeHolder1 = routeReg->lookup(origAddr, destAddr);
	if (routeHolder1)
	{
		const SmeSystemId& smeId1 = routeHolder1->route.smeSystemId;
		SmeType smeType1 = smeReg->getSmeBindType(smeId1);
		const RouteHolder* routeHolder2 = routeReg->lookup(destAddr, origAddr);
		if (routeHolder2)
		{
			const SmeSystemId& smeId2 = routeHolder2->route.smeSystemId;
			SmeType smeType2 = smeReg->getSmeBindType(smeId2);
			__trace2__("route: origAddr = %s, origSmeId = %s, destAias = %s, route to = %s, sme type = %d, back route to = %s, sme type = %d",
				str(origAddr).c_str(), origSmeId.c_str(), str(destAlias).c_str(),
				smeId1.c_str(), smeType1, smeId2.c_str(), smeType2);
		}
		else
		{
			__trace2__("route: origAddr = %s, origSmeId = %s, destAias = %s, route to = %s, sme type = %d, no back route",
				str(origAddr).c_str(), origSmeId.c_str(), str(destAlias).c_str(),
				smeId1.c_str(), smeType1);
		}
	}
	else
	{
		__trace2__("route: origAddr = %s, origSmeId = %s, destAias = %s, no route",
			str(origAddr).c_str(), origSmeId.c_str(), str(destAlias).c_str());
	}
}

bool ConfigUtil::checkAlias(const Address& addr)
{
	const Address alias = aliasReg->findAliasByAddress(addr);
	const Address addr2 = aliasReg->findAddressByAlias(alias);
	__trace2__("addr->alias->addr: %s -> %s -> %s",
		str(addr).c_str(), str(alias).c_str(), str(addr2).c_str());
	return (addr2 == addr);
}

}
}
}

