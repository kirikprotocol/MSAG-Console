#ifndef TEST_STORE_MESSAGE_STORE_INTEGRITY_TEST
#define TEST_STORE_MESSAGE_STORE_INTEGRITY_TEST

namespace smsc  {
namespace test  {
namespace store {

/**
 * Этот класс содержит все test cases необходимые для тестирования подсистемы
 * Message Store:
 * <ul>
 * <li>Сохранение SMS сообщений в БД.
 * <li>Обновление уже существующих в БД SMS сообщений.
 * <li>Удаление SMS сообщений из БД.
 * <li>(???)Создание записей для биллинга в таблице БД CBOSS.
 * </ul>
 * 
 * @author bryz
 */
class MessageStoreIntegrityTest
{
public:
	/**
	 * Выполняет все test cases.
	 */
	void executeAllTestCases();

	/**
	 * Сохраняет правильное SMS сообщение в базу.
	 * Только debug информация должна выводиться в лог.
	 */
	void storeCorrectSM();

	/**
	 * Сохраняет неправильное SMS сообщение в базу.
	 * Диагностика ошибки должна выводиться в лог.
	 */
	void storeIncorrectSM();

	/**
	 * Сохраняет в базу правильное SMS сообщение, затем корректно изменяет статус SM.
	 * Только debug информация должна выводиться в лог.
	 */
	void setCorrectSMStatus();

	/**
	 * Сохраняет в базу правильное SMS сообщение, затем пытается выставить некорректный статус SM.
	 * Диагностика ошибки должна выводиться в лог.
	 */
	void setIncorrectSMStatus();

	/**
	 * Сохраняет в базу правильное SMS сообщение, затем удаляет его.
	 * Только debug информация должна выводиться в лог.
	 */
	void deleteExistingSM();
	
	/**
	 * Пытается удалить несуществующее в базе SMS сообщение.
	 * Только debug информация должна выводиться в лог.
	 */
	void deleteNonExistingSM();
	
	/**
	 * Сохраняет в базу правильное SMS сообщение, затем загружает его.
	 * Только debug информация должна выводиться в лог.
	 */
	void loadExistingSM();

	/**
	 * Пытается загрузить несуществующее в базе SMS сообщение.
	 * Только debug информация должна выводиться в лог.
	 */
	void loadNonExistingSM();

private:
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_INTEGRITY_TEST */
