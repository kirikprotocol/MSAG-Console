#include "util/config/Manager.h"
#include "store/StoreManager.h"
#include "MessageStoreTestCases.hpp"
#include "test/util/CheckList.hpp"
#include "test/util/Util.hpp"
#include "test/util/TCResultFilter.hpp"
#include <iostream>

using namespace std;
using namespace smsc::sms;
using namespace smsc::store;
using namespace smsc::test::store;
using namespace smsc::test::util;
using namespace smsc::util::config;


void executeIntegrityTest(TCResultFilter* filter, int listSize)
{
	cout << ".";
	MessageStoreTestCases tc; //throws exception
	SMSId id[listSize];
	SMS sms[listSize];
	TCResultStack stack[listSize];

	//создание SM, 100%
	//cout << "Создание SM" << endl;
	for (int i = 0; i < listSize; i++)
	{
		TCResult* res = tc.storeCorrectSM(&id[i], &sms[i], RAND_TC);
		stack[i].push_back(res);
	}
	
	//сохранение правильного SM с параметрами похожими на уже существующий SM, 1/15
	//сохранение дублированного SM с отказом, 1/15
	//сохранение неправильного SM, 1/15
	//установка правильного статуса, 1/15
	//некорректное изменение статуса SM, 1/15
	//замещение SM, 1/15
	//некорректное замещение SM, 1/15
	//чтение SM, 8/15
	//cout << "Издевательство над существующими SM" << endl;
	for (TCSelector s(RAND_SET_TC, 15); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				for (int i = 0; i < listSize; i++)
				{
					SMSId newId;
					SMS newSMS;
					//создать
					TCResult* res1 = tc.storeCorrectSM(&newId, &newSMS,
						id[i], sms[i], RAND_TC);
					stack[i].push_back(res1);
					//прочитать
					TCResult* res2 = tc.loadExistentSM(id[i], sms[i]);
					stack[i].push_back(res2);
					//удалить
					TCResult* res3 = tc.deleteExistentSM(newId);
					stack[i].push_back(res3);
				}
				break;
			case 2:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.storeRejectDuplicateSM(sms[i]);
					stack[i].push_back(res);
				}
				break;
			case 3:
				{
					TCResult* res = tc.storeIncorrectSM(RAND_TC);
					filter->addResult(res);
					//stack[i].push_back(res);
				}
				break;
			case 4:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.setCorrectSMStatus(id[i], &sms[i], RAND_TC);
					stack[i].push_back(res);
				}
				break;
			case 5:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.setIncorrectSMStatus(id[i]);
					stack[i].push_back(res);
				}
				break;
			case 6:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.replaceCorrectSM(id[i], &sms[i], RAND_TC);
					stack[i].push_back(res);
				}
				break;
			case 7:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.replaceIncorrectSM(id[i], sms[i], RAND_TC);
					stack[i].push_back(res);
				}
				break;
		default: //case = 8..15
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.loadExistentSM(id[i], sms[i]);
					stack[i].push_back(res);
				}
		}
	}

	//удаление SM, 100%
	//cout << "Удаление SM" << endl;
	for (int i = 0; i < listSize; i++)
	{
		TCResult* res = tc.deleteExistentSM(id[i]);
		stack[i].push_back(res);
	}

	//изменение статуса несуществующих SM
	//замещение несуществующих SM
	//чтение несуществующих SM
	//удаление несуществующих SM
	//cout << "Издевательство над удаленными SM" << endl;
	for (TCSelector s(RAND_SET_TC, 4); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.setNonExistentSMStatus(id[i], RAND_TC);
					stack[i].push_back(res);
				}
				break;
			case 2:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.replaceNonExistentSM(id[i], RAND_TC);
					stack[i].push_back(res);
				}
				break;
			case 3:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.loadNonExistentSM(id[i], RAND_TC);
					stack[i].push_back(res);
				}
				break;
			case 4:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.deleteNonExistentSM(id[i], RAND_TC);
					stack[i].push_back(res);
				}
				break;
		}
	}

	//прочее
	//cout << "Прочее" << endl;
#ifdef ASSERT_THROW_IF_FAIL
	filter->addResult(tc.storeAssertSM(ALL_TC));
#endif

	//обработка результатов
	//cout << "Обработка результатов" << endl;
	for (int i = 0; i < listSize; i++)
	{
		filter->addResultStack(stack[i]);
	}
}

void saveCheckList(TCResultFilter* filter)
{
	cout << "Сохранение checklist" << endl;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("Message Store", "smsc::store");
	//заимплементированные тест кейсы
	cl.writeResult("Сохранение правильного SM",
		filter->getResults(TC_STORE_CORRECT_SM));
	cl.writeResult("Сохранение дублированного SM",
		filter->getResults(TC_STORE_REJECT_DUPLICATE_SM));
	cl.writeResult("Сохранение неправильного SM",
		filter->getResults(TC_STORE_INCORRECT_SM));
	cl.writeResult("Сохранение неправильного SM, проверка на assert",
		filter->getResults(TC_STORE_ASSERT_SM));
	cl.writeResult("Чтение существующего SM",
		filter->getResults(TC_LOAD_EXISTENT_SM));
	cl.writeResult("Чтение несуществующего SM",
		filter->getResults(TC_LOAD_NON_EXISTENT_SM));
	cl.writeResult("Удаление существующего SM",
		filter->getResults(TC_DELETE_EXISTENT_SM));
	cl.writeResult("Удаление несуществующего SM",
		filter->getResults(TC_DELETE_NON_EXISTENT_SM));
	cl.writeResult("Корректная замена существующего SM",
		filter->getResults(TC_REPLACE_CORRECT_SM));
	cl.writeResult("Замена существующего SM некорректными данными",
		filter->getResults(TC_REPLACE_INCORRECT_SM));
	cl.writeResult("Замена несуществующего SM",
		filter->getResults(TC_REPLACE_NON_EXISTENT_SM));
	
	//пока еще незаимплементированные тест кейсы
	cl.writeResult("Корректное изменение статуса SM",
		filter->getResults(TC_SET_CORRECT_SM_STATUS));
	cl.writeResult("Некорректное изменение статуса SM",
		filter->getResults(TC_SET_INCORRECT_SM_STATUS));
	cl.writeResult("Изменение статуса несуществующего SM",
		filter->getResults(TC_SET_NON_EXISTENT_SM_STATUS));
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

