#include "AliasManagerTestCases.hpp"
#include "util/Logger.h"

using log4cpp::Category;
using smsc::util::Logger;
using smsc::sms::Address;
using smsc::alias::AliasManager;
using smsc::test::sms::SmsUtil;
using smsc::test::core::AliasRegistry;
using smsc::test::core::TestAliasData;
using smsc::test::alias::AliasManagerTestCases;
using namespace smsc::test::util;

static Category& log = Logger::getCategory("AliasManagerTest");

void executeTest()
{
/*
addCorrectAliasException(2)->
findAliasByAddress(1){100}
*/
	AliasManager aliasMan;
	AliasManagerTestCases tc(&aliasMan);
	AliasRegistry aliasReg;

	Address alias, addr;
	SmsUtil::setupRandomCorrectAddress(&alias);
	SmsUtil::setupRandomCorrectAddress(&addr);

	TestAliasData data(alias, addr);
	cout << *tc.addCorrectAliasException(&data, 2) << endl;
	aliasReg.putAlias(data);

	/*
	TestAliasData data2(alias, addr);
	cout << *tc.addCorrectAliasException(&data, 1) << endl;
	aliasReg.putAlias(data2);
	*/

	cout << *tc.findAliasByAddress(aliasReg, addr) << endl;
/*
	cout << *tc.addCorrectAliasMatch(&data, 1) << endl;
	cout << *tc.addCorrectAliasNotMatchAddress(&data, 1) << endl;
	cout << *tc.addCorrectAliasNotMatchAlias(&data, 1) << endl;
	cout << *tc.addCorrectAliasException(&data, 1) << endl;
	cout << *tc.addIncorrectAlias() << endl;
	cout << *tc.deleteAliases() << endl;
	cout << *tc.findAliasByAddress(aliasReg, addr) << endl;
	cout << *tc.findAddressByAlias(aliasReg, alias) << endl;
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

