#include "AliasManagerTestCases.hpp"
#include "util/Logger.h"

using log4cpp::Category;
using smsc::util::Logger;
using smsc::sms::Address;
using smsc::alias::AliasInfo;
using smsc::alias::AliasManager;
using smsc::test::sms::SmsUtil;
using smsc::test::core::AliasRegistry;
using smsc::test::alias::AliasManagerTestCases;
using namespace smsc::test::util;

static Category& log = Logger::getCategory("AliasManagerTest");

void executeTest()
{
/*
*/
	AliasManager aliasMan;
	AliasRegistry aliasReg;
	AliasManagerTestCases tc(&aliasMan, &aliasReg, NULL);

	Address alias, addr;
	SmsUtil::setupRandomCorrectAddress(&alias);
	SmsUtil::setupRandomCorrectAddress(&addr);

	AliasInfo aliasInfo;
	aliasInfo.alias = alias;
	aliasInfo.addr = addr;
	tc.addCorrectAliasMatch(&aliasInfo, 9);

	AliasInfo aliasInfo2;
	aliasInfo2.alias = alias;
	aliasInfo2.addr = addr;
	tc.addCorrectAliasMatch(&aliasInfo, 9);

	/*
	AliasInfo aliasInfo3;
	aliasInfo3.alias = alias;
	aliasInfo3.addr = addr;
	cout << *tc.addCorrectAliasMatch(&aliasInfo, 9) << endl;
	*/
	
	tc.findAliasByAddress(addr);
	//cout << *tc.findAddressByAlias(alias) << endl;
/*
	cout << *tc.addCorrectAliasMatch(&aliasInfo, 1) << endl;
	cout << *tc.addCorrectAliasNotMatchAddress(&aliasInfo, 1) << endl;
	cout << *tc.addCorrectAliasNotMatchAlias(&aliasInfo, 1) << endl;
	cout << *tc.deleteAliases() << endl;
	cout << *tc.findAliasByAddress(addr) << endl;
	cout << *tc.findAddressByAlias(alias) << endl;
	cout << *tc.iterateAliases() << endl;
*/

}

int main(int argc, char* argv[])
{
	try
	{
		//Manager::init("config.xml");
		executeTest();
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

