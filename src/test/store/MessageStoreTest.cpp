#include "util/debug.h"
#include "logger/Logger.h"
#include "util/config/Manager.h"
#include "store/StoreManager.h"
#include "MessageStoreTestCases.hpp"
#include "MessageStoreCheckList.hpp"
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

#define __new__ \
	id.push_back(new SMSId()); \
	sms.push_back(new SMS());
	
void executeTest(MessageStoreTestCases& tc)
{
	Category& log = Logger::getCategory("smsc.test.store.Test");

	vector<SMSId*> id;
	vector<SMS*> sms;
	
	__new__;
	/*
	tc.storeCorrectSms(id.back(), sms.back(), 4);
	tc.loadExistentSms(*id.back(), *sms.back());
	tc.replaceCorrectSms(*id.back(), sms.back(), 2);
	tc.loadExistentSms(*id.back(), *sms.back());
	//tc.changeExistentSmsStateEnrouteToFinal(*id.back(), sms.back(), 2);
	//tc.loadExistentSms(*id.back(), *sms.back());
	*/

	tc.storeCorrectSms(id.back(), sms.back(), 10);
	tc.loadExistentSms(*id.back(), *sms.back());
	tc.replaceIncorrectSms(*id.back(), *sms.back(), 3);
	tc.loadExistentSms(*id.back(), *sms.back());
	tc.replaceCorrectSms(*id.back(), sms.back(), 5);
	tc.loadExistentSms(*id.back(), *sms.back());
	tc.changeExistentSmsStateEnrouteToFinal(*id.back(), sms.back(), 6);
	tc.loadExistentSms(*id.back(), *sms.back());

/*
	tc.storeCorrectSms(id.back(), sms.back(), RAND_TC);
	tc.storeSimilarSms(id.back(), sms.back(), const SMSId existentId, const SMS& existentSms, RAND_TC);
	tc.storeDuplicateSms(id.back()p, sms.back()p, const SMSId existentId, const SMS& existentSms);
	tc.storeRejectDuplicateSms(const SMS& existentSms);
	tc.storeReplaceCorrectSms(id.back(), SMS* existentSms);
	tc.storeReplaceSmsInFinalState(id.back(), sms.back(), const SMS& existentSms);
	tc.storeIncorrectSms(RAND_TC);
	tc.storeAssertSms(RAND_TC);
	tc.changeExistentSmsStateEnrouteToEnroute(*id.back(), sms.back(), RAND_TC);
	tc.changeExistentSmsStateEnrouteToFinal(*id.back(), sms.back(), RAND_TC);
	tc.changeFinalSmsStateToAny(*id.back(), RAND_TC);
	tc.replaceCorrectSms(*id.back(), sms.back(), RAND_TC);
	tc.replaceIncorrectSms(*id.back(), const SMS& sms, RAND_TC);
	tc.replaceFinalSms(*id.back(), const SMS& sms);
	tc.loadExistentSms(*id.back(), const SMS& sms);
	tc.loadNonExistentSms(*id.back());
	tc.deleteExistentSms(*id.back());
	tc.deleteNonExistentSms(*id.back());
	tc.checkReadyForRetrySms(const vector<SMSId*>& ids, const vector<SMS*>& sms, RAND_TC);
*/
}

int main(int argc, char* argv[])
{
	try
	{
		Manager::init("config.xml");
		StoreManager::startup(Manager::getInstance());
		MessageStoreCheckList chkList;
		MessageStoreTestCases tc(StoreManager::getMessageStore(), false, &chkList); //throws exception
		executeTest(tc);
		chkList.save();
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

