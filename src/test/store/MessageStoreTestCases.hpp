#ifndef TEST_STORE_MESSAGE_STORE_TEST_CASES
#define TEST_STORE_MESSAGE_STORE_TEST_CASES

#include "sms/sms.h"
#include "store/MessageStore.h"
#include "test/util/Util.hpp"
#include "test/util/BaseTestCases.hpp"
#include <exception>
#include <vector>

namespace smsc  {
namespace test  {
namespace store {

using std::vector;
using log4cpp::Category;
using smsc::sms::SMSId;
using smsc::sms::SMS;
using smsc::store::MessageStore;
using smsc::test::util::BaseTestCases;
using smsc::test::util::CheckList;

/**
 * Этот класс содержит все test cases необходимые для тестирования подсистемы
 * Message Store.
 * 
 * @author bryz
 */
class MessageStoreTestCases : BaseTestCases
{
public:
	/**
	 * Инициализирует подсистему Message Store.
	 * @exception std::exception если инициализация не прошла.
	 */
	MessageStoreTestCases(MessageStore* msgStore, bool loadTest = false,
		CheckList* chkList = NULL);

	/**
	 * Деинициализирует подсистему Message Store.
	 */
	~MessageStoreTestCases() {}

	/**
	 * Сохранение правильного sms.
	 */
	void storeCorrectSms(SMSId* id, SMS* sms, int num);

	/**
	 * Сохранение правильного sms, но с параметрами очень похожими на уже 
	 * существующий sms.
	 */
	void storeSimilarSms(SMSId* id, SMS* sms,
		const SMSId existentId, const SMS& existentSms, int num);

	/**
	 * Сохранение дублированного sms.
	 */
	void storeDuplicateSms(SMSId* idp, SMS* smsp, const SMSId existentId,
		const SMS& existentSms);

	/**
	 * Сохранение дублированного sms с отказом.
	 */
	void storeRejectDuplicateSms(const SMS& existentSms);

	/**
	 * Сохранение корректного sms с замещением уже существующего.
	 */
	void storeReplaceCorrectSms(SMSId* id, SMS* existentSms);

	/**
	 * Сохранение sms с замещением существующего sms финальном состоянии.
	 */
	void storeReplaceSmsInFinalState(SMSId* id, SMS* sms,
		const SMS& existentSms);

	/**
	 * Сохранение неправильного sms.
	 */
	void storeIncorrectSms(int num);

	/**
	 * Сохранение неправильного sms с проверкой на assert.
	 */
	void storeAssertSms(int num);

	/**
	 * Обновление статуса sms в состоянии ENROUTE.
	 */
	void changeExistentSmsStateEnrouteToEnroute(const SMSId id, SMS* sms, int num);

	/**
	 * Перевод sms из ENROUTE в финальное состояние.
	 */
	void changeExistentSmsStateEnrouteToFinal(const SMSId id, SMS* sms, int num);

	/**
	 * Перевод несуществующего sms или sms в финальном состоянии в любое другое состояние.
	 */
	void changeFinalSmsStateToAny(const SMSId id, int num);

	/**
	 * Корректное обновление существующего sms.
	 */
	void replaceCorrectSms(const SMSId id, SMS* sms, int num);

	/**
	 * Некорректное обновление существующего или обновление несуществующего sms.
	 */
	void replaceIncorrectSms(const SMSId id, const SMS& sms, int num);

	/**
	 * Обновление несуществующего sms или sms в финальном состоянии.
	 */
	void replaceFinalSms(const SMSId id, const SMS& sms);

	/**
	 * Чтение существующего sms.
	 */
	void loadExistentSms(const SMSId id, const SMS& sms);

	/**
	 * Чтение несуществующего sms.
	 */
	void loadNonExistentSms(const SMSId id);

	/**
	 * Удаление существующего sms.
	 * Этот тест кейс не соответствует требованиям к системе, поскольку в 
	 * реальной работе удалением сообщений (переносом в архив) занимается 
	 * архиватор.
	 */
	void deleteExistentSms(const SMSId id);
	
	/**
	 * Удаление несуществующего sms.
	 * Этот тест кейс не соответствует требованиям к системе, поскольку в 
	 * реальной работе удалением сообщений (переносом в архив) занимается 
	 * архиватор.
	 */
	void deleteNonExistentSms(const SMSId id);

	/**
	 * Получение списка sms для повторной доставки.
	 */
	void checkReadyForRetrySms(const vector<SMSId*>& ids,
		const vector<SMS*>& sms, int num);

	/**
	 * Удаление существующих sms ожидающих доставки на определенный номер.
	 */
	void deleteExistentWaitingSMByNumber();
	
	/**
	 * Удаление несуществующих sms ожидающих доставки на определенный номер.
	 */
	void deleteNonExistentWaitingSMByNumber();
	
	/**
	 * Загрузка непустого списка sms ожидающих доставки на определенный номер.
	 */
	void loadExistentWaitingSMByDestinationNumber();

	/**
	 * Загрузка пустого списка sms ожидающих доставки на определенный номер.
	 */
	void loadNonExistentWaitingSMByDestinationNumber();

	/**
	 * Загрузка непустого архива sms доставленных на определенный номер.
	 */
	void loadExistentSMArchieveByDestinationNumber();

	/**
	 * Загрузка пустого архива sms доставленных на определенный номер.
	 */
	void loadNonExistentSMArchieveByDestinationNumber();

	/**
	 * Загрузка непустого архива sms доставленных с определенного номера.
	 */
	void loadExistentSMArchieveByOriginatingNumber();
	
	/**
	 * Загрузка пустого архива sms доставленных с определенного номера.
	 */
	void loadNonExistentSMArchieveByOriginatingNumber();

	/**
	 * Просмотр непустой статистики причин недоставки сообщений.
	 */
	void getExistentSMDeliveryFailureStatistics();

	/**
	 * Просмотр пустой статистики причин недоставки сообщений.
	 */
	void getNonExistentSMDeliveryFailureStatistics();

private:
	MessageStore* msgStore;
	CheckList* chkList;
	bool loadTest;
	uint64_t mask;
	bool check;

	virtual Category& getLog();
	void checkNextRetryTime(const vector<SMSId*>& ids,
		const vector<SMS*>& sms, time_t& minNextTime,
		time_t& middleNextTime, time_t& maxNextTime);
	void compareReadyForRetrySmsList(const vector<SMSId*>& ids, 
		const vector<SMS*>& sms, time_t time, int shift);
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_TEST_CASES */

