#include "util/config/Manager.h"
#include "store/StoreManager.h"
#include "core/synchronization/Event.hpp"
#include "MessageStoreTestCases.hpp"
#include "test/util/CheckList.hpp"
#include "test/util/Util.hpp"
#include "test/util/TCResultFilter.hpp"
#include <iostream>

using namespace smsc::sms; //SMSId, SMS
using namespace smsc::test::store; //constants, MessageStoreTestCases
using namespace smsc::test::util; //constants, TCResult, TCResultStack, ...
using std::vector;
using std::ostringstream;
using log4cpp::Category;
using smsc::util::Logger;
using smsc::util::config::Manager;
using smsc::store::StoreManager;
using smsc::core::synchronization::Event;

static Category& log = Logger::getCategory("smsc.test.store.IntegrityTest");

#define PREPARE_FOR_NEW_SMS \
	id.push_back(new SMSId()); \
	sms.push_back(new SMS()); \
	stack.push_back(new TCResultStack());
	
void executeIntegrityTest(MessageStoreTestCases& tc,
	TCResultFilter* filter, int listSize)
{
	cout << ".";
	log.debug("*** start ***\n");
	vector<SMSId*> id;
	vector<SMS*> sms;
	vector<TCResultStack*> stack;
	static executeTestCounter = 0;

	//Сохранение правильного sms, 1/1
	for (int i = 0; i < listSize; i++)
	{
		PREPARE_FOR_NEW_SMS
		TCResult* res = tc.storeCorrectSms(id.back(), sms.back(), RAND_TC);
		stack.back()->push_back(res);
	}
	
	//Сохранение правильного sms с параметрами похожими на уже существующий sms, 1/15
	//Сохранение дублированного sms, 1/15
	//Сохранение дублированного sms с отказом, 1/15
	//Сохранение корректного sms с замещением уже существующегоб 1/15
	//Сохранение неправильного sms, 1/15
	//Обновление статуса sms в состоянии ENROUTE, 1/15
	//Корректное обновление существующего sms, 1/15
    //Некорректное обновление существующего sms, 1/15
	//Чтение существующего sms, 7/15
	bool duplicatesOk = rand0(1); //взаимоисключающие тест кейсы
	for (TCSelector s(RAND_SET_TC, 15); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				for (int i = 0; i < listSize; i++)
				{
					PREPARE_FOR_NEW_SMS
					TCResult* res = tc.storeCorrectSms(id.back(), sms.back(),
						*id[i], *sms[i], RAND_TC);
					stack.back()->push_back(res);
				}
				break;
			case 2:
				for (int i = 0; duplicatesOk && i < listSize; i++)
				{
					PREPARE_FOR_NEW_SMS
					TCResult* res = tc.storeDuplicateSms(id.back(), sms.back(),
						*id[i], *sms[i]);
					stack.back()->push_back(res);
				}
				break;
			case 3:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.storeRejectDuplicateSms(*sms[i]);
					stack[i]->push_back(res);
				}
				break;
			case 4:
				for (int i = 0; !duplicatesOk && i < id.size(); i++)
				{
					TCResult* res = tc.storeReplaceCorrectSms(id[i], sms[i]);
					stack[i]->push_back(res);
				}
				break;
			case 5:
				{
					TCResult* res = tc.storeIncorrectSms(RAND_TC);
					filter->addResult(res);
					//stack[i]->push_back(res);
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
					TCResult* res = tc.changeExistentSmsStateEnrouteToEnroute(
						*id[i], sms[i], RAND_TC);
					stack[i]->push_back(res);
				}
				break;
			case 7:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.replaceCorrectSms(*id[i], sms[i], RAND_TC);
					stack[i]->push_back(res);
				}
				break;
			case 8:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.replaceIncorrectSms(*id[i], *sms[i], RAND_TC);
					stack[i]->push_back(res);
				}
				break;
			default: //case = 9..15
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.loadExistentSms(*id[i], *sms[i]);
					stack[i]->push_back(res);
				}
		}
	}

	//Получение списка sms для повторной доставки
	if (!executeTestCounter)
	{
		TCResult* res = tc.checkReadyForRetrySms(id, sms, RAND_TC);
		filter->addResult(res);
	}

	//добавить паузу, чтобы SMS::lastTime отличалось от предыдущего
	if (!rand0(3))
	{
		static Event evt;
		evt.Wait(1100);
		log.debug("*** wait ***\n");
	}
	//Перевод sms из ENROUTE в финальное состояние, 1/1
	for (int i = 0; i < id.size(); i++)
	{
		TCResult* res = tc.changeExistentSmsStateEnrouteToFinal(*id[i], 
			sms[i], RAND_TC);
		stack[i]->push_back(res);
	}
	
	//Перевод sms в финальном состоянии в любое другое состояние, 1/5
	//Сохранение sms с замещением существующего sms финальном состоянии, 1/5
	//Обновление sms в финальном состоянии, 1/5
	//Чтение существующего sms, 2/5
	for (TCSelector s(RAND_SET_TC, 5); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.changeFinalSmsStateToAny(*id[i], RAND_TC);
					stack[i]->push_back(res);
				}
				break;
			case 2:
				for (int i = 0; i < listSize; i++)
				{
					PREPARE_FOR_NEW_SMS
					TCResult* res1 = tc.storeReplaceSmsInFinalState(id.back(),
						sms.back(), *sms[i]);
					stack.back()->push_back(res1);
					//обязательно перевести созданное сообщение в финальной состояние
					TCResult* res2 = tc.changeExistentSmsStateEnrouteToFinal(
						*id.back(), sms.back(), RAND_TC);
					stack.back()->push_back(res2);
				}
				break;
			case 3:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.replaceFinalSms(*id[i], *sms[i]);
					stack[i]->push_back(res);
				}
				break;
			default: //4..5
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.loadExistentSms(*id[i], *sms[i]);
					stack[i]->push_back(res);
				}
		}
	}

	//Получение списка sms для повторной доставки
	if (!executeTestCounter)
	{
		TCResult* res = tc.checkReadyForRetrySms(id, sms, RAND_TC);
		filter->addResult(res);
	}

	//Удаление существующего sms, 1/1
	for (int i = 0; i < id.size(); i++)
	{
		TCResult* res = tc.deleteExistentSms(*id[i]);
		stack[i]->push_back(res);
	}

	//Перевод несуществующего sms в любое состояние, 1/4
	//Некорректное обновление несуществующего sms, 1/4
	//Чтение несуществующего sms, 1/4
	//Удаление несуществующего sms, 1/4
	for (TCSelector s(RAND_SET_TC, 4); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.changeFinalSmsStateToAny(*id[i], RAND_TC);
					stack[i]->push_back(res);
				}
				break;
			case 2:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.replaceIncorrectSms(*id[i], *sms[i], RAND_TC);
					//TCResult* res = tc.replaceFinalSms(*id[i], *sms[i]);
					stack[i]->push_back(res);
				}
				break;
			case 3:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.loadNonExistentSms(*id[i]);
					stack[i]->push_back(res);
				}
				break;
			case 4:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.deleteNonExistentSms(*id[i]);
					stack[i]->push_back(res);
				}
				break;
		}
	}

	//Получение списка sms для повторной доставки
	if (!executeTestCounter)
	{
		TCResult* res = tc.checkReadyForRetrySms(vector<SMSId*>(),
			vector<SMS*>(), RAND_TC);
		filter->addResult(res);
	}

//Сохранение неправильного sms с проверкой на assert
#ifdef ASSERT_THROW_IF_FAIL
	TCResult* res = tc.storeAssertSms(ALL_TC);
	filter->addResult(res);
#endif

	//обработка результатов
	for (int i = 0; i < stack.size(); i++)
	{
		filter->addResultStack(*stack[i]);
	}

	//очистка памяти
	for (int i = 0; i < id.size(); i++)
	{
		delete id[i];
		delete sms[i];
		delete stack[i];
	}
	executeTestCounter++;
}

void saveCheckList(TCResultFilter* filter)
{
	cout << "Сохранение checklist" << endl;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("Message Store", "smsc::store");
	//имплементированные тест кейсы
	cl.writeResult("Сохранение правильного sms",
		filter->getResults(TC_STORE_CORRECT_SMS));
	cl.writeResult("Сохранение дублированного sms",
		filter->getResults(TC_STORE_DUPLICATE_SMS));
	cl.writeResult("Сохранение дублированного sms с отказом",
		filter->getResults(TC_STORE_REJECT_DUPLICATE_SMS));
	cl.writeResult("Сохранение корректного sms с замещением уже существующего",
		filter->getResults(TC_STORE_REPLACE_CORRECT_SMS));
	cl.writeResult("Сохранение sms с замещением существующего sms финальном состоянии",
		filter->getResults(TC_STORE_REPLACE_SMS_IN_FINAL_STATE));
	cl.writeResult("Сохранение неправильного sms",
		filter->getResults(TC_STORE_INCORRECT_SMS));
	cl.writeResult("Сохранение неправильного sms с проверкой на assert",
		filter->getResults(TC_STORE_ASSERT_SMS));
	cl.writeResult("Обновление статуса sms в состоянии ENROUTE",
		filter->getResults(TC_CHANGE_EXISTENT_SMS_STATE_ENROUTE_TO_ENROUTE));
	cl.writeResult("Перевод sms из ENROUTE в финальное состояние",
		filter->getResults(TC_CHANGE_EXISTENT_SMS_STATE_ENROUTE_TO_FINAL));
	cl.writeResult("Перевод несуществующего sms или sms в финальном состоянии в любое другое состояние",
		filter->getResults(TC_CHANGE_FINAL_SMS_STATE_TO_ANY));
	cl.writeResult("Корректное обновление существующего sms",
		filter->getResults(TC_REPLACE_CORRECT_SMS));
	cl.writeResult("Некорректное обновление существующего или обновление несуществующего sms",
		filter->getResults(TC_REPLACE_INCORRECT_SMS));
	cl.writeResult("Обновление несуществующего sms или sms в финальном состоянии",
		filter->getResults(TC_REPLACE_FINAL_SMS));
	cl.writeResult("Чтение существующего sms",
		filter->getResults(TC_LOAD_EXISTENT_SMS));
	cl.writeResult("Чтение несуществующего sms",
		filter->getResults(TC_LOAD_NON_EXISTENT_SMS));
	cl.writeResult("Удаление существующего sms",
		filter->getResults(TC_DELETE_EXISTENT_SMS));
	cl.writeResult("Удаление несуществующего sms",
		filter->getResults(TC_DELETE_NON_EXISTENT_SMS));
	cl.writeResult("Получение списка sms для повторной доставки",
		filter->getResults(TC_CHECK_READY_FOR_RETRY_SMS));

	//пока еще незаимплементированные тест кейсы
	cl.writeResult("Удаление существующих SM ожидающих доставки на определенный номер",
		filter->getResults(TC_DELETE_EXISTENT_WAITING_SM_BY_NUMBER));
	cl.writeResult("Удаление несуществующих SM ожидающих доставки на определенный номер",
		filter->getResults(TC_DELETE_NON_EXISTENT_WAITING_SM_BY_NUMBER));
	cl.writeResult("Загрузка непустого списка SM ожидающих доставки на определенный номер",
		filter->getResults(TC_LOAD_EXISTENT_WAITING_SM_BY_DESTINATION_NUMBER));
	cl.writeResult("Загрузка пустого списка SM ожидающих доставки на определенный номер",
		filter->getResults(TC_LOAD_NON_EXISTENT_WAITING_SM_BY_DESTINATION_NUMBER));
	cl.writeResult("Загрузка непустого архива SM доставленных на определенный номер",
		filter->getResults(TC_LOAD_EXISTENT_SM_ARCHIEVE_BY_DESTINATION_NUMBER));
	cl.writeResult("Загрузка пустого архива SM доставленных на определенный номер",
		filter->getResults(TC_LOAD_NON_EXISTENT_SM_ARCHIEVE_BY_DESTINATION_NUMBER));
	cl.writeResult("Загрузка непустого архива SM доставленных с определенного номера",
		filter->getResults(TC_LOAD_EXISTENT_SM_ARCHIEVE_BY_ORIGINATING_NUMBER));
	cl.writeResult("Загрузка пустого архива SM доставленных с определенного номера",
		filter->getResults(TC_LOAD_NON_EXISTENT_SM_ARCHIEVE_BY_ORIGINATING_NUMBER));
	cl.writeResult("Просмотр непустой статистики причин недоставки сообщений",
		filter->getResults(TC_GET_EXISTENT_SM_DELIVERY_FAILURE_STATISTICS));
	cl.writeResult("Просмотр пустой статистики причин недоставки сообщений",
		filter->getResults(TC_GET_NON_EXISTENT_SM_DELIVERY_FAILURE_STATISTICS));
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
		//StoreManager::stopArchiver();
		TCResultFilter* filter = new TCResultFilter();
		MessageStoreTestCases tc(StoreManager::getMessageStore()); //throws exception
		for (int i = 0; i < numCycles; i++)
		{
			executeIntegrityTest(tc, filter, numSms);
		}
		saveCheckList(filter);
		delete filter;
		StoreManager::shutdown();
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

