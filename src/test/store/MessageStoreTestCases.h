#ifndef TEST_STORE_MESSAGE_STORE_TEST_CASES
#define TEST_STORE_MESSAGE_STORE_TEST_CASES

namespace smsc  {
namespace test  {
namespace store {

/**
 * Этот класс содержит все test cases необходимые для тестирования подсистемы
 * Message Store.
 * 
 * @author bryz
 */
class MessageStoreTestCases
{
public:
	/**
	 * Сохранение правильного SM.
	 * Только debug информация должна выводиться в лог.
	 */
	bool storeCorrectSM(int num = -1);

	/**
	 * Сохранение неправильного SM.
	 * Диагностика ошибки должна выводиться в лог.
	 */
	bool storeIncorrectSM();

	/**
	 * Корректное изменение статуса SM.
	 * Сохраняет в базу правильное SMS сообщение, затем корректно изменяет статус SM.
	 * Только debug информация должна выводиться в лог.
	 */
	bool setCorrectSMStatus();

	/**
	 * Некорректное изменение статуса SM.
	 * Сохраняет в базу правильное SMS сообщение, затем пытается выставить некорректный статус SM.
	 * Диагностика ошибки должна выводиться в лог.
	 */
	bool setIncorrectSMStatus();

	/**
	 * Изменение статуса несуществующего SM.
	 * Диагностика ошибки должна выводиться в лог.
	 */
	bool setNonExistentSMStatus();

	/**
	 * Корректное обновление существующего SM.
	 * Только debug информация должна выводиться в лог.
	 */
	bool updateCorrectExistentSM();

	/**
	 * Обновление существующего SM некорректными данными.
	 * Диагностика ошибки должна выводиться в лог.
	 */
	bool updateIncorrectExistentSM();
	
	/**
	 * Обновление несуществующего SM.
	 * Диагностика ошибки должна выводиться в лог.
	 */
	bool updateNonExistentSM();

	/**
	 * Удаление существующего SM.
	 * Сохраняет в базу правильное SMS сообщение, затем удаляет его.
	 * Только debug информация должна выводиться в лог.
	 */
	bool deleteExistingSM();
	
	/**
	 * Удаление несуществующего SM.
	 * Только debug информация должна выводиться в лог.
	 */
	bool deleteNonExistingSM();
	
	/**
	 * Чтение существующего SM.
	 * Сохраняет в базу правильное SMS сообщение, затем загружает его.
	 * Только debug информация должна выводиться в лог.
	 */
	bool loadExistingSM();

	/**
	 * Чтение несуществующего SM.
	 * Только debug информация должна выводиться в лог.
	 */
	bool loadNonExistingSM();

	/**
	 * Создание записи для начисления оплаты.
	 */
	bool createBillingRecord();
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_TEST_CASES */
