#include "util/config/Manager.h"
#include "store/StoreManager.h"
#include "MessageStoreTestCases.hpp"
#include "test/util/CheckList.hpp"
#include "test/util/Util.hpp"
#include "test/util/TCResultFilter.hpp"
#include <iostream>
#include <sstream>

using namespace smsc::sms; //SMSId, SMS
using namespace smsc::test::store; //constants, MessageStoreTestCases
using namespace smsc::test::util; //constants, TCResult, TCResultStack, ...
using std::vector;
using std::ostringstream;
using log4cpp::Category;
using smsc::util::Logger;
using smsc::util::config::Manager;
using smsc::store::StoreManager;

#define PREPARE_FOR_NEW_SMS \
	id.push_back(new SMSId()); \
	sms.push_back(new SMS()); \
	stack.push_back(new TCResultStack());
	
void debug(TCResult* res)
{
	static Category& log = Logger::getCategory("smsc.test.store.IntegrityTest");
	if (res)
	{
		ostringstream os;
		os << *res << endl;
		log.debug("%s", os.str().c_str());
	}
}

void executeIntegrityTest(TCResultFilter* filter, int listSize)
{
	cout << ".";
	MessageStoreTestCases tc; //throws exception
	vector<SMSId*> id;
	vector<SMS*> sms;
	vector<TCResultStack*> stack;

	//Сохранение правильного sms, 1/1
	for (int i = 0; i < listSize; i++)
	{
		PREPARE_FOR_NEW_SMS
		TCResult* res = tc.storeCorrectSms(id.back(), sms.back(), RAND_TC);
		debug(res);
		stack.back()->push_back(res);
	}
	
	//Сохранение правильного sms с параметрами похожими на уже существующий sms, 1/15
	//Сохранение дублированного sms с отказом, 1/15
	//Сохранение корректного sms с замещением уже существующегоб 1/15
	//Сохранение неправильного sms, 1/15
	//Обновление статуса sms в состоянии ENROUTE, 1/15
	//Корректное обновление существующего sms, 1/15
    //Некорректное обновление существующего sms, 1/15
	//Чтение существующего sms, 8/15
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
					debug(res);
					stack.back()->push_back(res);
				}
				break;
			case 2:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.storeRejectDuplicateSms(*sms[i]);
					debug(res);
					stack[i]->push_back(res);
				}
				break;
			case 3:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.storeReplaceCorrectSms(*id[i], sms[i]);
					debug(res);
					stack[i]->push_back(res);
				}
				break;
			case 4:
				{
					TCResult* res = tc.storeIncorrectSms(RAND_TC);
					debug(res);
					filter->addResult(res);
					//stack[i]->push_back(res);
				}
				break;
			case 5:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.changeExistentSmsStateEnrouteToEnroute(
						*id[i], sms[i], RAND_TC);
					debug(res);
					stack[i]->push_back(res);
				}
				break;
			case 6:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.replaceCorrectSms(*id[i], sms[i], RAND_TC);
					debug(res);
					stack[i]->push_back(res);
				}
				break;
			case 7:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.replaceIncorrectSms(*id[i], *sms[i], RAND_TC);
					debug(res);
					stack[i]->push_back(res);
				}
				break;
			default: //case = 8..15
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.loadExistentSms(*id[i], *sms[i]);
					debug(res);
					stack[i]->push_back(res);
				}
		}
	}

	//Перевод sms из ENROUTE в финальное состояние, 1/1
	for (int i = 0; i < id.size(); i++)
	{
		TCResult* res = tc.changeExistentSmsStateEnrouteToFinal(*id[i], 
			sms[i], RAND_TC);
		debug(res);
		stack[i]->push_back(res);
	}
	
	//Перевод sms в финальном состоянии в любое другое состояние, 1/4
	//Сохранение sms с замещением существующего sms финальном состоянии, 1/4
	//Чтение существующего sms, 2/4
	for (TCSelector s(RAND_SET_TC, 4); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.changeFinalSmsStateToAny(*id[i], RAND_TC);
					debug(res);
					stack[i]->push_back(res);
				}
				break;
			case 2:
				for (int i = 0; i < listSize; i++)
				{
					PREPARE_FOR_NEW_SMS
					TCResult* res = tc.storeReplaceSmsInFinalState(id.back(),
						sms.back(), *id[i], *sms[i]);
					debug(res);
					stack[i]->push_back(res);
				}
				break;
			default: //3..4
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.loadExistentSms(*id[i], *sms[i]);
					debug(res);
					stack[i]->push_back(res);
				}
		}
	}

	//Удаление существующего sms, 1/1
	for (int i = 0; i < id.size(); i++)
	{
		TCResult* res = tc.deleteExistentSms(*id[i]);
		debug(res);
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
					debug(res);
					stack[i]->push_back(res);
				}
				break;
			case 2:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.replaceIncorrectSms(*id[i], *sms[i], RAND_TC);
					debug(res);
					stack[i]->push_back(res);
				}
				break;
			case 3:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.loadNonExistentSms(*id[i]);
					debug(res);
					stack[i]->push_back(res);
				}
				break;
			case 4:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.deleteNonExistentSms(*id[i]);
					debug(res);
					stack[i]->push_back(res);
				}
				break;
		}
	}

//Сохранение неправильного sms с проверкой на assert
#ifdef ASSERT_THROW_IF_FAIL
	TCResult* res = tc.storeAssertSms(ALL_TC);
	filter->addResult(res);
	debug(res);
#endif

	//обработка результатов
	for (int i = 0; i < listSize; i++)
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
}

void saveCheckList(TCResultFilter* filter)
{
	cout << "Сохранение checklist" << endl;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("Message Store", "smsc::store");
	//имплементированные тест кейсы
	cl.writeResult("Сохранение правильного sms",
		filter->getResults(TC_STORE_CORRECT_SMS));
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
	cl.writeResult("Чтение существующего sms",
		filter->getResults(TC_LOAD_EXISTENT_SMS));
	cl.writeResult("Чтение несуществующего sms",
		filter->getResults(TC_LOAD_NON_EXISTENT_SMS));
	cl.writeResult("Удаление существующего sms",
		filter->getResults(TC_DELETE_EXISTENT_SMS));
	cl.writeResult("Удаление несуществующего sms",
		filter->getResults(TC_DELETE_NON_EXISTENT_SMS));
	
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
	cl.writeResult("Создание записи для начисления оплаты",
		filter->getResults(TC_CREATE_BILLING_RECORD));
}

/**
 * Выполняет тестирование целостности данных (Data and Database Integrity 
 * Testing) и выводит результат по тест кейсам в checklist.
 */
int main(int argc, char* argv[])
{
	try
	{
		Manager::init("config.xml");
		StoreManager::startup(Manager::getInstance());
		StoreManager::stopArchiver();
		TCResultFilter* filter = new TCResultFilter();
		for (int i = 0; i < 50; i++)
		{
			executeIntegrityTest(filter, 1);
		}
		for (int i = 0; i < 50; i++)
		{
			executeIntegrityTest(filter, 5);
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

