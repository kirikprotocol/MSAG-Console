#include "MessageStoreTestCases.hpp"
#include "test/util/CheckList.hpp"
#include "test/util/Util.hpp"
#include "test/util/TCResultFilter.hpp"
#include <iostream>

using namespace std;
using namespace smsc::sms;
using namespace smsc::test::store;
using namespace smsc::test::util;

TCResultFilter* newFilter()
{
    TCResultFilter* filter = new TCResultFilter();
	filter->registerTC(TC_STORE_CORRECT_SM,
		"Сохранение правильного SM");
	filter->registerTC(TC_STORE_INCORRECT_SM,
		"Сохранение неправильного SM");
	filter->registerTC(TC_SET_CORRECT_SM_STATUS,
		"Корректное изменение статуса SM");
	filter->registerTC(TC_SET_INCORRECT_SM_STATUS,
		"Некорректное изменение статуса SM");
	filter->registerTC(TC_SET_NON_EXISTENT_SM_STATUS,
		"Изменение статуса несуществующего SM");
	filter->registerTC(TC_UPDATE_CORRECT_EXISTENT_SM,
		"Корректное обновление существующего SM");
	filter->registerTC(TC_UPDATE_INCORRECT_EXISTENT_SM,
		"Обновление существующего SM некорректными данными");
	filter->registerTC(TC_UPDATE_NON_EXISTENT_SM,
		"Обновление несуществующего SM");
	filter->registerTC(TC_DELETE_EXISTENT_SM,
		"Удаление существующего SM");
	filter->registerTC(TC_DELETE_NON_EXISTENT_SM,
		"Удаление несуществующего SM");
	filter->registerTC(TC_DELETE_EXISTENT_WAITING_SM_BY_NUMBER,
		"Удаление существующих SM ожидающих доставки на определенный номер");
	filter->registerTC(TC_DELETE_NON_EXISTENT_WAITING_SM_BY_NUMBER,
		"Удаление несуществующих SM ожидающих доставки на определенный номер");
	filter->registerTC(TC_LOAD_EXISTENT_SM,
		"Чтение существующего SM");
	filter->registerTC(TC_LOAD_NONEXISTENT_SM,
		"Чтение несуществующего SM");
	filter->registerTC(TC_LOAD_EXISTENT_WAITING_SM_BY_DESTINATION_NUMBER,
		"Загрузка непустого списка SM ожидающих доставки на определенный номер");
	filter->registerTC(TC_LOAD_NON_EXISTENT_WAITING_SM_BY_DESTINATION_NUMBER,
		"Загрузка пустого списка SM ожидающих доставки на определенный номер");
	filter->registerTC(TC_LOAD_EXISTENT_SM_ARCHIEVE_BY_DESTINATION_NUMBER,
		"Загрузка непустого архива SM доставленных на определенный номер");
	filter->registerTC(TC_LOAD_NON_EXISTENT_SM_ARCHIEVE_BY_DESTINATION_NUMBER,
		"Загрузка пустого архива SM доставленных на определенный номер");
	filter->registerTC(TC_LOAD_EXISTENT_SM_ARCHIEVE_BY_ORIGINATING_NUMBER,
		"Загрузка непустого архива SM доставленных с определенного номера");
	filter->registerTC(TC_LOAD_NON_EXISTENT_SM_ARCHIEVE_BY_ORIGINATING_NUMBER,
		"Загрузка пустого архива SM доставленных с определенного номера");
	filter->registerTC(TC_GET_EXISTENT_SM_DELIVERY_FAILURE_STATISTICS,
		"Просмотр непустой статистики причин недоставки сообщений");
	filter->registerTC(TC_GET_NON_EXISTENT_SM_DELIVERY_FAILURE_STATISTICS,
		"Просмотр пустой статистики причин недоставки сообщений");
	filter->registerTC(TC_CREATE_BILLING_RECORD,
		"Создание записи для начисления оплаты");
	return filter;
}

/**
 * Выполняет тестирование целостности данных (Data and Database Integrity 
 * Testing) и выводит результат по test cases в checklist.
 *
 * @param argc
 * @param argv
 * @return 
 */
int main(int argc, char* argv[])
{
	MessageStoreTestCases tc; //throws StoreException
    TCResultFilter* filter = newFilter();

	//выполнение тестов
	SMSId id;
	SMS sms;
	for (int i = 0; i < 10; i++)
	{
cout << "i = " << i << endl;
		TCResultStack stack;
		stack.push_back(tc.storeCorrectSM(&id, &sms, RAND_TC));
		stack.push_back(tc.loadExistentSM(id, sms));
		filter->addResultStack(stack);
	}
	filter->addResult(*tc.storeIncorrectSM(sms, ALL_TC));
	filter->addResult(*tc.loadNonExistentSM());
	
	//сохранить checklist
cout << "Save checklist" << endl;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("Message Store", "smsc::store");
	cl.writeResult(*filter);


	/*
	//Изменение статуса SM
	cl.writeResult("Корректное изменение статуса SM", tc.setCorrectSMStatus());
	cl.writeResult("Некорректное изменение статуса SM", 
		tc.setIncorrectSMStatus());
	cl.writeResult("Изменение статуса несуществующего SM", 
		tc.setNonExistentSMStatus());
	
	//Обновление SM
	cl.writeResult("Корректное обновление существующего SM", 
		tc.updateCorrectExistentSM());
	cl.writeResult("Обновление существующего SM некорректными данными", 
		tc.updateIncorrectExistentSM());
	cl.writeResult("Обновление несуществующего SM", tc.updateNonExistentSM());
	
	//Удаление SM
	cl.writeResult("Удаление существующего SM", tc.deleteExistentSM());
	cl.writeResult("Удаление несуществующего SM", tc.deleteNonExistentSM());
	
	//Создание записи для начисления оплаты
	cl.writeResult("Создание записи для начисления оплаты", 
		tc.createBillingRecord());
	*/

	return 0;
}

