#include "MessageStoreTestCases.h"
#include "../util/CheckList.h"

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
	MessageStoreTestCases tc;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("Message Store");
	
	//Сохранение SM
	cl.writeResult("Сохранение правильного SM", tc.storeCorrectSM());
	cl.writeResult("Сохранение неправильного SM", tc.storeIncorrectSM());
	
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
	cl.writeResult("Удаление существующего SM", tc.deleteExistingSM());
	cl.writeResult("Удаление несуществующего SM", tc.deleteNonExistingSM());
	
	//Чтение SM
	cl.writeResult("Чтение существующего SM", tc.loadExistingSM());
	cl.writeResult("Чтение несуществующего SM", tc.loadNonExistingSM());
	
	//Создание записи для начисления оплаты
	cl.writeResult("Создание записи для начисления оплаты", 
		tc.createBillingRecord());

	return 0;
}

