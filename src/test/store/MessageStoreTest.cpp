#include "util/debug.h"
#include "util/Logger.h"
#include "util/config/Manager.h"
#include "store/StoreManager.h"
#include "MessageStoreTestCases.hpp"
#include "test/util/TestTaskManager.hpp"
#include "test/util/TCResultFilter.hpp"
#include "test/sms/SmsUtil.hpp"
#include <iostream>
#include <sstream>
#include <sys/timeb.h>

#define PRINT(STR) cout << #STR << " = " << (STR) << endl

using namespace std;
using namespace smsc::sms;
using namespace smsc::store;
using namespace smsc::util::config;
using namespace smsc::test::store;
using namespace smsc::test::util;
using std::ostringstream;
using log4cpp::Category;
using smsc::util::Logger;
using smsc::test::sms::SmsUtil;

void executeTest()
{
	Category& log = Logger::getCategory("smsc.test.store.Test");
	MessageStoreTestCases tc;

	SMSId id, id2, id3, id4, id5;
	SMS sms, sms2, sms3, sms4, sms5;
	
/*
storeCorrectSms(13)->
replaceIncorrectSms(3){3}
*/
	cout << *tc.storeCorrectSms(&id, &sms, 11) << endl;
	cout << *tc.changeExistentSmsStateEnrouteToFinal(id, &sms, 6) << endl;
	cout << *tc.deleteExistentSms(id) << endl;
/*
	cout << *tc.storeCorrectSms(&id, &sms, 1) << endl;
	cout << *tc.storeCorrectSms(&id2, &sms2, id, sms, 1) << endl;
	cout << *tc.storeRejectDuplicateSms(sms) << endl;
	cout << *tc.storeReplaceCorrectSms(&id, &sms) << endl;
	cout << *tc.storeReplaceSmsInFinalState(&id2, &sms2, id, sms) << endl;
	cout << *tc.storeIncorrectSms(1) << endl;
	cout << *tc.storeAssertSms(1) << endl;
	cout << *tc.changeExistentSmsStateEnrouteToEnroute(id, &sms, 1) << endl;
	cout << *tc.changeExistentSmsStateEnrouteToFinal(id, &sms, 1) << endl;
	cout << *tc.changeFinalSmsStateToAny(id, 1) << endl;
	cout << *tc.replaceCorrectSms(id, &sms, 1) << endl;
	cout << *tc.replaceIncorrectSms(id, sms, 1) << endl;
	cout << *tc.replaceFinalSms(id, sms) << endl;
	cout << *tc.loadExistentSms(id, sms) << endl;
	cout << *tc.loadNonExistentSms(id) << endl;
	cout << *tc.deleteExistentSms(id) << endl;
	cout << *tc.deleteNonExistentSms(id) << endl;
*/
}

int main(int argc, char* argv[])
{
	try
	{
		Manager::init("config.xml");
		StoreManager::startup(Manager::getInstance());
		executeTest();
		StoreManager::shutdown();
		exit(0);
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
		exit(-1);
	}
	return 0;
}

