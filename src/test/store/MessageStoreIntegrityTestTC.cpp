#include "MessageStoreTestCases.hpp"
#include "test/util/CheckList.hpp"
#include "test/util/Util.hpp"
#include "test/util/TCResultFilter.hpp"
#include <iostream>

using namespace std;
using namespace smsc::sms;
using namespace smsc::test::store;
using namespace smsc::test::util;

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
    TCResultFilter filter;

	//регистрация описаний test cases
cout << "Register test cases" << endl;
	filter.registerTC(TC_STORE_CORRECT_SM, "Сохранение правильного SM");
	filter.registerTC(TC_STORE_INCORRECT_SM, "Сохранение неправильного SM");
	filter.registerTC(TC_LOAD_EXISTENT_SM, "Чтение существующего SM");
	filter.registerTC(TC_LOAD_NONEXISTENT_SM, "Чтение несуществующего SM");
	
	//выполнение тестов
	SMSId id;
	SMS sms;
	for (int i = 0; i < 10; i++)
	{
cout << "i = " << i << endl;
		TCResultStack stack;
		stack.push_back(tc.storeCorrectSM(&id, &sms, RAND_TC));
		stack.push_back(tc.loadExistentSM(id, sms));
		filter.addResultStack(stack);
	}
	filter.addResult(*tc.storeIncorrectSM(sms, RAND_TC));
	filter.addResult(*tc.loadNonExistentSM());
	
	//сохранить checklist
cout << "Save checklist" << endl;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("Message Store");
	cl.writeResult(filter);


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

