#ifndef TEST_STORE_MESSAGE_STORE_USE_CASES
#define TEST_STORE_MESSAGE_STORE_USE_CASES

#include "MessageStoreTestCases.h"

namespace smsc  {
namespace test  {
namespace store {

/**
 * Этот класс содержит все use cases необходимые для тестирования подсистемы
 * Message Store.
 * 
 * @author bryz
 */
class MessageStoreUseCases
{
public:
	/**
	 * Сохранение SM.
	 */
	bool storeSM();

	/**
	 * Изменение статуса SM.
	 */
	bool setSMStatus();

	/**
	 * Обновление SM.
	 */
	bool updateSM();

	/**
	 * Удаление SM.
	 */
	bool deleteSM();
	
	/**
	 * Чтение SM.
	 */
	bool loadSM();

	/**
	 * Создание записи для начисления оплаты.
	 */
	bool createBillingRecord();

private:
	MessageStoreTestCases tc;
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_USE_CASES */
