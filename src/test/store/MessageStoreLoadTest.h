#ifndef TEST_STORE_MESSAGE_STORE_LOAD_TEST
#define TEST_STORE_MESSAGE_STORE_LOAD_TEST

namespace smsc  {
namespace test  {
namespace store {

/**
 * Ётот класс содержит единственный test case дл€ тестировани€ подсистемы 
 * Message Store при нормальных и высоких нагрузках.
 * @author bryz
 */
class MessageStoreLoadTest
{
public:
	/**
	 * «апускает тест на исполнение. “ест исполн€ет в бесконечном цикле test 
	 * cases:
	 * <ul>
	 * <li>—охранение SMS сообщений (20 правильных, 1 неправильное) в базу.
	 * <li>»зменение статуса SMS сообщений (20 корректных, 1 некорректное).
	 * <li>«агрузка SMS сообщений из базы (1 существующее).
	 * </ul>
	 * 
	 * @param numThreads количество одновременных процессов. ¬ каждом процессе
	 * сообщени€ непрерывно (без пауз) сохран€ютс€ в базу.
	 */
	void startTest(int numThreads);

	/**
	 * ѕрекращает выполнение теста.
	 * 
	 * @return количество SMS сообщений сохран€емых в базу в секунду.
	 */
	int stopTest();
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_LOAD_TEST */
