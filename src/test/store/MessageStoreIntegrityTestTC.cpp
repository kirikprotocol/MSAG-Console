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

TCResultFilter* executeIntegrityTest()
{
	MessageStoreTestCases tc; //throws exception
    TCResultFilter* filter = new TCResultFilter();

	//создаю SM, затем сразу читаю, замещаю, опять читаю и удаляю
	cout << "Step 1" << endl;
	for (int i = 0; i < 100; i++)
	{
		SMSId id;
		SMS sms;
		TCResultStack stack;
		stack.push_back(tc.storeCorrectSM(&id, &sms, RAND_TC));
		stack.push_back(tc.loadExistentSM(id, sms));

		stack.push_back(tc.replaceCorrectSM(id, &sms, RAND_TC));
		stack.push_back(tc.loadExistentSM(id, sms));

        stack.push_back(tc.setCorrectSMStatus(id, &sms, RAND_TC));
		stack.push_back(tc.loadExistentSM(id, sms));

		stack.push_back(tc.storeDuplicateSM(sms, RAND_TC));
		stack.push_back(tc.storeIncorrectSM(RAND_TC));
		stack.push_back(tc.replaceIncorrectSM(id, sms, RAND_TC));
		stack.push_back(tc.setIncorrectSMStatus(id));
		stack.push_back(tc.loadExistentSM(id, sms));

		stack.push_back(tc.deleteExistentSM(id));

		stack.push_back(tc.setNonExistentSMStatus(id, RAND_TC));
		stack.push_back(tc.deleteNonExistentSM(id, RAND_TC));
		stack.push_back(tc.replaceNonExistentSM(id, RAND_TC));
		stack.push_back(tc.loadNonExistentSM(id, RAND_TC));
		filter->addResultStack(stack);
	}
	
	//сначала создаю список, потом читаю, замещаю, читаю и удаляю
	const int listSize = 20;
	for (int j = 0; j < 3; j++)
	{
		cout << "Step " << (j + 2) << endl;
		SMSId id[listSize];
		SMS sms[listSize];
		TCResultStack stack[listSize];
		//создание SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.storeCorrectSM(&id[i], &sms[i], RAND_TC);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//чтение SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.loadExistentSM(id[i], sms[i]);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//некорректное изменение статуса SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.setIncorrectSMStatus(id[i]);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//некорректное замещение SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.replaceIncorrectSM(id[i], sms[i], RAND_TC);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//сохранение дублированного SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.storeDuplicateSM(sms[i], RAND_TC);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//чтение SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.loadExistentSM(id[i], sms[i]);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//установка правильного статуса
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.setCorrectSMStatus(id[i], &sms[i], RAND_TC);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//чтение SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.loadExistentSM(id[i], sms[i]);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//замещение SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.replaceCorrectSM(id[i], &sms[i], RAND_TC);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//чтение SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.loadExistentSM(id[i], sms[i]);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//удаление SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.deleteExistentSM(id[i]);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//чтение несуществующих SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.loadNonExistentSM(id[i], RAND_TC);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//замещение несуществующих SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.replaceNonExistentSM(id[i], RAND_TC);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//удаление несуществующих SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.deleteNonExistentSM(id[i], RAND_TC);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//изменение статуса несуществующих SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.setNonExistentSMStatus(id[i], RAND_TC);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//обработка результатов
		for (int i = 0; i < listSize; i++)
		{
			filter->addResultStack(stack[i]);
		}
	}

	//прочее
	cout << "Step 3" << endl;
	filter->addResult(tc.storeIncorrectSM(ALL_TC));
#ifdef ASSERT_THROW_IF_FAIL
	filter->addResult(tc.storeAssertSM(ALL_TC));
#endif
	return filter;
}

void saveCheckList(TCResultFilter* filter)
{
	cout << "Сохранение checklist" << endl;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("Message Store", "smsc::store");
	//заимплементированные test cases
	cl.writeResult("Сохранение правильного SM",
		filter->getResults(TC_STORE_CORRECT_SM));
	cl.writeResult("Сохранение дублированного SM",
		filter->getResults(TC_STORE_DUPLICATE_SM));
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
	
	//пока еще незаимплементированные test cases
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
 * Testing) и выводит результат по test cases в checklist.
 */
int main(int argc, char* argv[])
{
	try
	{
		Manager::init("config.xml");
		StoreManager::startup(Manager::getInstance());
		TCResultFilter* filter = executeIntegrityTest();
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

