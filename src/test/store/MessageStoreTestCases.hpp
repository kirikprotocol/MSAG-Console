#ifndef TEST_STORE_MESSAGE_STORE_TEST_CASES
#define TEST_STORE_MESSAGE_STORE_TEST_CASES

#include "sms/sms.h"
#include "store/MessageStore.h"
#include "test/util/Util.hpp"
#include <exception>

namespace smsc  {
namespace test  {
namespace store {

using log4cpp::Category;
using smsc::sms::SMSId;
using smsc::sms::SMS;
using smsc::store::MessageStore;
using smsc::test::util::TCResult;

//implemented
const char* const TC_STORE_CORRECT_SMS = "storeCorrectSms";
const char* const TC_STORE_DUPLICATE_SMS = "storeDuplicateSms";
const char* const TC_STORE_REJECT_DUPLICATE_SMS = "storeRejectDuplicateSms";
const char* const TC_STORE_REPLACE_CORRECT_SMS = "storeReplaceCorrectSms";
const char* const TC_STORE_REPLACE_SMS_IN_FINAL_STATE = "storeReplaceSmsInFinalState";
const char* const TC_STORE_INCORRECT_SMS = "storeIncorrectSms";
const char* const TC_STORE_ASSERT_SMS = "storeAssertSms";
const char* const TC_CHANGE_EXISTENT_SMS_STATE_ENROUTE_TO_ENROUTE = 
	"changeExistentSmsStateEnrouteToEnroute";
const char* const TC_CHANGE_EXISTENT_SMS_STATE_ENROUTE_TO_FINAL = 
	"changeExistentSmsStateEnrouteToFinal";
const char* const TC_CHANGE_FINAL_SMS_STATE_TO_ANY = "changeFinalSmsStateToAny";
const char* const TC_REPLACE_CORRECT_SMS = "replaceCorrectSms";
const char* const TC_REPLACE_INCORRECT_SMS = "replaceIncorrectSms";
const char* const TC_REPLACE_FINAL_SMS = "replaceFinalSms";
const char* const TC_LOAD_EXISTENT_SMS = "loadExistentSms";
const char* const TC_LOAD_NON_EXISTENT_SMS = "loadNonExistentSms";
const char* const TC_DELETE_EXISTENT_SMS = "deleteExistentSms";
const char* const TC_DELETE_NON_EXISTENT_SMS = "deleteNonExistentSms";
//not implemented yet
const char* const TC_DELETE_EXISTENT_WAITING_SM_BY_NUMBER = 
	"deleteExistentWaitingSMByNumber";
const char* const TC_DELETE_NON_EXISTENT_WAITING_SM_BY_NUMBER = 
	"deleteNonExistentWaitingSMByNumber";
const char* const TC_LOAD_EXISTENT_WAITING_SM_BY_DESTINATION_NUMBER = 
	"loadExistentWaitingSMByDestinationNumber";
const char* const TC_LOAD_NON_EXISTENT_WAITING_SM_BY_DESTINATION_NUMBER = 
	"loadNonExistentWaitingSMByDestinationNumber";
const char* const TC_LOAD_EXISTENT_SM_ARCHIEVE_BY_DESTINATION_NUMBER = 
	"loadExistentSMArchieveByDestinationNumber";
const char* const TC_LOAD_NON_EXISTENT_SM_ARCHIEVE_BY_DESTINATION_NUMBER = 
	"loadNonExistentSMArchieveByDestinationNumber";
const char* const TC_LOAD_EXISTENT_SM_ARCHIEVE_BY_ORIGINATING_NUMBER = 
	"loadExistentSMArchieveByOriginatingNumber";
const char* const TC_LOAD_NON_EXISTENT_SM_ARCHIEVE_BY_ORIGINATING_NUMBER = 
	"loadNonExistentSMArchieveByOriginatingNumber";
const char* const TC_GET_EXISTENT_SM_DELIVERY_FAILURE_STATISTICS = 
	"getExistentSMDeliveryFailureStatistics";
const char* const TC_GET_NON_EXISTENT_SM_DELIVERY_FAILURE_STATISTICS = 
	"getNonExistentSMDeliveryFailureStatistics";
const char* const TC_CREATE_BILLING_RECORD = "createBillingRecord";

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
	 * Инициализирует подсистему Message Store.
	 * @exception std::exception если инициализация не прошла.
	 */
	MessageStoreTestCases();

	/**
	 * Деинициализирует подсистему Message Store.
	 */
	~MessageStoreTestCases() {}

	/**
	 * Сохранение правильного sms.
	 */
	TCResult* storeCorrectSms(SMSId* id, SMS* sms, int num);

	/**
	 * Сохранение правильного sms, но с параметрами очень похожими на уже 
	 * существующий sms.
	 */
	TCResult* storeCorrectSms(SMSId* id, SMS* sms,
		const SMSId existentId, const SMS& existentSms, int num);

	/**
	 * Сохранение дублированного sms.
	 */
	TCResult* storeDuplicateSms(SMSId* idp, SMS* smsp, const SMSId existentId,
		const SMS& existentSms);

	/**
	 * Сохранение дублированного sms с отказом.
	 */
	TCResult* storeRejectDuplicateSms(const SMS& existentSms);

	/**
	 * Сохранение корректного sms с замещением уже существующего.
	 */
	TCResult* storeReplaceCorrectSms(SMSId* id, SMS* existentSms);

	/**
	 * Сохранение sms с замещением существующего sms финальном состоянии.
	 */
	TCResult* storeReplaceSmsInFinalState(SMSId* id, SMS* sms,
		const SMS& existentSms);

	/**
	 * Сохранение неправильного sms.
	 */
	TCResult* storeIncorrectSms(int num);

	/**
	 * Сохранение неправильного sms с проверкой на assert.
	 */
	TCResult* storeAssertSms(int num);

	/**
	 * Обновление статуса sms в состоянии ENROUTE.
	 */
	TCResult* changeExistentSmsStateEnrouteToEnroute(const SMSId id, SMS* sms, int num);

	/**
	 * Перевод sms из ENROUTE в финальное состояние.
	 */
	TCResult* changeExistentSmsStateEnrouteToFinal(const SMSId id, SMS* sms, int num);

	/**
	 * Перевод несуществующего sms или sms в финальном состоянии в любое другое состояние.
	 */
	TCResult* changeFinalSmsStateToAny(const SMSId id, int num);

	/**
	 * Корректное обновление существующего sms.
	 */
	TCResult* replaceCorrectSms(const SMSId id, SMS* sms, int num);

	/**
	 * Некорректное обновление существующего или обновление несуществующего sms.
	 */
	TCResult* replaceIncorrectSms(const SMSId id, const SMS& sms, int num);

	/**
	 * Обновление несуществующего sms или sms в финальном состоянии.
	 */
	TCResult* replaceFinalSms(const SMSId id, const SMS& sms);

	/**
	 * Чтение существующего sms.
	 */
	TCResult* loadExistentSms(const SMSId id, const SMS& sms);

	/**
	 * Чтение несуществующего sms.
	 */
	TCResult* loadNonExistentSms(const SMSId id);

	/**
	 * Удаление существующего sms.
	 * Этот тест кейс не соответствует требованиям к системе, поскольку в 
	 * реальной работе удалением сообщений (переносом в архив) занимается 
	 * архиватор.
	 */
	TCResult* deleteExistentSms(const SMSId id);
	
	/**
	 * Удаление несуществующего sms.
	 * Этот тест кейс не соответствует требованиям к системе, поскольку в 
	 * реальной работе удалением сообщений (переносом в архив) занимается 
	 * архиватор.
	 */
	TCResult* deleteNonExistentSms(const SMSId id);

	/**
	 * Удаление существующих sms ожидающих доставки на определенный номер.
	 */
	TCResult* deleteExistentWaitingSMByNumber();
	
	/**
	 * Удаление несуществующих sms ожидающих доставки на определенный номер.
	 */
	TCResult* deleteNonExistentWaitingSMByNumber();
	
	/**
	 * Загрузка непустого списка sms ожидающих доставки на определенный номер.
	 */
	TCResult* loadExistentWaitingSMByDestinationNumber();

	/**
	 * Загрузка пустого списка sms ожидающих доставки на определенный номер.
	 */
	TCResult* loadNonExistentWaitingSMByDestinationNumber();

	/**
	 * Загрузка непустого архива sms доставленных на определенный номер.
	 */
	TCResult* loadExistentSMArchieveByDestinationNumber();

	/**
	 * Загрузка пустого архива sms доставленных на определенный номер.
	 */
	TCResult* loadNonExistentSMArchieveByDestinationNumber();

	/**
	 * Загрузка непустого архива sms доставленных с определенного номера.
	 */
	TCResult* loadExistentSMArchieveByOriginatingNumber();
	
	/**
	 * Загрузка пустого архива sms доставленных с определенного номера.
	 */
	TCResult* loadNonExistentSMArchieveByOriginatingNumber();

	/**
	 * Просмотр непустой статистики причин недоставки сообщений.
	 */
	TCResult* getExistentSMDeliveryFailureStatistics();

	/**
	 * Просмотр пустой статистики причин недоставки сообщений.
	 */
	TCResult* getNonExistentSMDeliveryFailureStatistics();

	/**
	 * Создание записи для начисления оплаты.
	 */
	TCResult* createBillingRecord();

private:
	static Category& log;
	MessageStore* msgStore;

	void error();
	void debug(const TCResult* res);
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_TEST_CASES */

