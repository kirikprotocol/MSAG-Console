#include "util/config/Manager.h"
#include "store/StoreManager.h"
#include "core/synchronization/Event.hpp"
#include "MessageStoreTestCases.hpp"
#include "MessageStoreCheckList.hpp"
#include "test/util/Util.hpp"
#include <iostream>

using namespace smsc::sms; //SMSId, SMS
using namespace smsc::test::store; //constants, MessageStoreTestCases
using namespace smsc::test::util; //constants, ...
using std::vector;
using std::ostringstream;
using log4cpp::Category;
using smsc::util::Logger;
using smsc::util::config::Manager;
using smsc::store::StoreManager;
using smsc::core::synchronization::Event;

static Category& log = Logger::getCategory("smsc.test.store.IntegrityTest");

#define __prepare_for_new_sms__ \
	id.push_back(new SMSId()); \
	sms.push_back(new SMS());
	
void executeIntegrityTest(MessageStoreTestCases& tc, int listSize)
{
	cout << ".";
	log.debug("*** start ***\n");
	vector<SMSId*> id;
	vector<SMS*> sms;
	static int executeTestCounter = 0;

	for (int i = 0; i < listSize; i++)
	{
		__prepare_for_new_sms__
		tc.storeCorrectSms(id.back(), sms.back(), RAND_TC);
	}
	
	bool duplicatesOk = rand0(1); //взаимоисключающие тест кейсы
	for (TCSelector s(RAND_SET_TC, 20); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				for (int i = 0; i < listSize; i++)
				{
					__prepare_for_new_sms__
					tc.storeSimilarSms(id.back(), sms.back(),
						*id[i], *sms[i], RAND_TC);
				}
				break;
			case 2:
				for (int i = 0; duplicatesOk && i < listSize; i++)
				{
					__prepare_for_new_sms__
					tc.storeDuplicateSms(id.back(), sms.back(),
						*id[i], *sms[i]);
				}
				break;
			case 3:
				for (int i = 0; i < id.size(); i++)
				{
					tc.storeRejectDuplicateSms(*sms[i]);
				}
				break;
			case 4:
				for (int i = 0; !duplicatesOk && i < id.size(); i++)
				{
					tc.storeReplaceCorrectSms(id[i], sms[i]);
				}
				break;
			case 5:
				{
					tc.storeIncorrectSms(RAND_TC);
				}
				break;
			case 6:
				//добавить паузу, чтобы SMS::lastTime отличалось от предыдущего
				if (!rand0(3))
				{
					static Event evt;
					evt.Wait(1100);
					log.debug("*** wait ***\n");
				}
				for (int i = 0; i < id.size(); i++)
				{
					tc.changeExistentSmsStateEnrouteToEnroute(
						*id[i], sms[i], RAND_TC);
				}
				break;
			case 7:
				for (int i = 0; i < id.size(); i++)
				{
					tc.replaceCorrectSms(*id[i], sms[i], RAND_TC);
				}
				break;
			case 8:
				for (int i = 0; i < id.size(); i++)
				{
					tc.replaceIncorrectSms(*id[i], *sms[i], RAND_TC);
				}
				break;
			case 9:
				tc.checkReadyForRetrySms(id, sms, RAND_TC);
				break;
			case 10:
				tc.checkReadyForDeliverySms(id, sms, RAND_TC);
				break;
			case 11:
				tc.checkReadyForCancelSms(id, sms, RAND_TC);
				break;
			case 12:
				tc.checkConcatInitInfo(id, sms);
				break;
			case 13:
				for (int i = 0; i < id.size(); i++)
				{
					tc.changeSmsConcatSequenceNumber(*id[i], sms[i]);
				}
				break;
			default: //case = 14..20
				for (int i = 0; i < id.size(); i++)
				{
					tc.loadExistentSms(*id[i], *sms[i]);
				}
		}
	}
	
	//добавить паузу, чтобы SMS::lastTime отличалось от предыдущего
	if (!rand0(3))
	{
		static Event evt;
		evt.Wait(1100);
		log.debug("*** wait ***\n");
	}
	for (int i = 0; i < id.size(); i++)
	{
		tc.changeExistentSmsStateEnrouteToFinal(*id[i], 
			sms[i], RAND_TC);
	}
	
	//после перевода в финальное состояние
	tc.checkConcatInitInfo(id, sms);
	
	for (TCSelector s(RAND_SET_TC, 12); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				for (int i = 0; i < id.size(); i++)
				{
					tc.changeFinalSmsStateToAny(*id[i], RAND_TC);
				}
				break;
			case 2:
				for (int i = 0; i < listSize; i++)
				{
					__prepare_for_new_sms__
					tc.storeReplaceSmsInFinalState(id.back(), sms.back(), *sms[i]);
					//обязательно перевести созданное сообщение в финальной состояние
					tc.changeExistentSmsStateEnrouteToFinal(*id.back(),
						sms.back(), RAND_TC);
				}
				break;
			case 3:
				for (int i = 0; i < id.size(); i++)
				{
					tc.replaceFinalSms(*id[i], *sms[i]);
				}
				break;
			case 4:
				tc.checkReadyForRetrySms(id, sms, RAND_TC);
				break;
			case 5:
				tc.checkReadyForDeliverySms(id, sms, RAND_TC);
				break;
			case 6:
				tc.checkReadyForCancelSms(id, sms, RAND_TC);
				break;
			case 7:
				tc.checkConcatInitInfo(id, sms);
				break;
			case 8:
				for (int i = 0; i < id.size(); i++)
				{
					tc.changeSmsConcatSequenceNumber(*id[i], sms[i]);
				}
				break;
			default: //9..12
				for (int i = 0; i < id.size(); i++)
				{
					tc.loadExistentSms(*id[i], *sms[i]);
				}
		}
	}

	for (int i = 0; i < id.size(); i++)
	{
		tc.deleteExistentSms(*id[i]);
	}

	for (TCSelector s(RAND_SET_TC, 4); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				for (int i = 0; i < id.size(); i++)
				{
					tc.changeFinalSmsStateToAny(*id[i], RAND_TC);
				}
				break;
			case 2:
				for (int i = 0; i < id.size(); i++)
				{
					tc.replaceIncorrectSms(*id[i], *sms[i], RAND_TC);
					//tc.replaceFinalSms(*id[i], *sms[i]);
				}
				break;
			case 3:
				for (int i = 0; i < id.size(); i++)
				{
					tc.loadNonExistentSms(*id[i]);
				}
				break;
			case 4:
				for (int i = 0; i < id.size(); i++)
				{
					tc.deleteNonExistentSms(*id[i]);
				}
				break;
		}
	}

//Сохранение неправильного sms с проверкой на assert
#ifdef ASSERT_THROW_IF_FAIL
	tc.storeAssertSms(ALL_TC);
#endif

	//очистка памяти
	for (int i = 0; i < id.size(); i++)
	{
		delete id[i];
		delete sms[i];
	}
	executeTestCounter++;
}

/**
 * Выполняет тестирование целостности данных (Data and Database Integrity 
 * Testing) и выводит результат по тест кейсам в checklist.
 */
int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		cout << "Usage: MessageStoreIntegrityTestTC <numCycles> <numSms>" << endl;
		exit(0);
	}

	const int numCycles = atoi(argv[1]);
	const int numSms = atoi(argv[2]);
	try
	{
		Manager::init("config.xml");
		StoreManager::startup(Manager::getInstance());
		StoreManager::stopArchiver();
		MessageStoreCheckList chkList;
		MessageStoreTestCases tc(StoreManager::getMessageStore(), false,
			&chkList); //throws exception
		for (int i = 0; i < numCycles; i++)
		{
			executeIntegrityTest(tc, numSms);
		}
		cout << endl;
		StoreManager::shutdown();
		chkList.save();
		chkList.saveHtml();
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

