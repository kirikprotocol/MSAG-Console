#include "AliasManagerTestCases.hpp"
#include "util/Logger.h"

using log4cpp::Category;
using smsc::util::Logger;
using smsc::sms::Address;
using smsc::alias::AliasManager;
using smsc::test::sms::SmsUtil;
using smsc::test::core::TestAliasData;
using smsc::test::alias::AliasManagerTestCases;
using namespace smsc::test::util;

static Category& log = Logger::getCategory("AliasManagerTest");

void executeTest()
{
/*
addCorrectAliasException(3)->
addCorrectAliasMatch(10){100}
*/
	AliasManager aliasMan;
	AliasManagerTestCases tc(&aliasMan);

	Address alias, addr;
	SmsUtil::setupRandomCorrectAddress(&alias);
	SmsUtil::setupRandomCorrectAddress(&addr);

	TestAliasData data(alias, addr);
	cout << *tc.addCorrectAliasException(&data, 3) << endl;

	TestAliasData data2(alias, addr);
	cout << *tc.addCorrectAliasMatch(&data2, 10) << endl;
/*
	cout << *tc.addCorrectAliasMatch(&data, 1) << endl;
	cout << *tc.addCorrectAliasNotMatchAddress(&data, 1) << endl;
	cout << *tc.addCorrectAliasNotMatchAlias(&data, 1) << endl;
	cout << *tc.addCorrectAliasException(&data, 1) << endl;
	cout << *tc.addIncorrectAlias() << endl;
	cout << *tc.deleteAliases() << endl;
	cout << *tc.findAliasByAddress(const AliasRegistry& aliasReg,
		const Address& addr) << endl;
	cout << *tc.findAddressByAlias(const AliasRegistry& aliasReg,
		const Address& alias) << endl;
	cout << *tc.iterateAliases(const AliasRegistry& aliasReg) << endl;
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

