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
using smsc::sms::Address;
using smsc::store::MessageStore;
using smsc::test::util::BaseTestCases;
using smsc::test::util::CheckList;

/**
 * Ётот класс содержит все test cases необходимые дл€ тестировани€ подсистемы
 * Message Store.
 * 
 * @author bryz
 */
class MessageStoreTestCases : BaseTestCases
{
public:
	/**
	 * »нициализирует подсистему Message Store.
	 * @exception std::exception если инициализаци€ не прошла.
	 */
	MessageStoreTestCases(MessageStore* msgStore, bool loadTest = false,
		CheckList* chkList = NULL);

	/**
	 * ƒеинициализирует подсистему Message Store.
	 */
	~MessageStoreTestCases() {}

	/**
	 * —охранение правильного sms.
	 */
	void storeCorrectSms(SMSId* id, SMS* sms, int num);

	/**
	 * —охранение правильного sms, но с параметрами очень похожими на уже 
	 * существующий sms.
	 */
	void storeSimilarSms(SMSId* id, SMS* sms,
		const SMSId existentId, const SMS& existentSms, int num);

	/**
	 * —охранение дублированного sms.
	 */
	void storeDuplicateSms(SMSId* idp, SMS* smsp, const SMSId existentId,
		const SMS& existentSms);

	/**
	 * —охранение дублированного sms с отказом.
	 */
	void storeRejectDuplicateSms(const SMS& existentSms);

	/**
	 * —охранение корректного sms с замещением уже существующего.
	 */
	void storeReplaceCorrectSms(SMSId* id, SMS* existentSms);

	/**
	 * —охранение sms с замещением существующего sms финальном состо€нии.
	 */
	void storeReplaceSmsInFinalState(SMSId* id, SMS* sms,
		const SMS& existentSms);

	/**
	 * —охранение неправильного sms.
	 */
	void storeIncorrectSms(int num);

	/**
	 * —охранение неправильного sms с проверкой на assert.
	 */
	void storeAssertSms(int num);

	/**
	 * ќбновление статуса sms в состо€нии ENROUTE.
	 */
	void changeExistentSmsStateEnrouteToEnroute(const SMSId id, SMS* sms, int num);

	/**
	 * ѕеревод sms из ENROUTE в финальное состо€ние.
	 */
	void changeExistentSmsStateEnrouteToFinal(const SMSId id, SMS* sms, int num);

	/**
	 * ѕеревод несуществующего sms или sms в финальном состо€нии в любое другое состо€ние.
	 */
	void changeFinalSmsStateToAny(const SMSId id, int num);

	/**
	 *  орректное обновление существующего sms.
	 */
	void replaceCorrectSms(const SMSId id, SMS* sms, int num);

	/**
	 * Ќекорректное обновление существующего или обновление несуществующего sms.
	 */
	void replaceIncorrectSms(const SMSId id, const SMS& sms, int num);

	/**
	 * ќбновление несуществующего sms или sms в финальном состо€нии.
	 */
	void replaceFinalSms(const SMSId id, const SMS& sms);

	/**
	 * „тение существующего sms.
	 */
	void loadExistentSms(const SMSId id, const SMS& sms);

	/**
	 * „тение несуществующего sms.
	 */
	void loadNonExistentSms(const SMSId id);

	/**
	 * ”даление существующего sms.
	 * Ётот тест кейс не соответствует требовани€м к системе, поскольку в 
	 * реальной работе удалением сообщений (переносом в архив) занимаетс€ 
	 * архиватор.
	 */
	void deleteExistentSms(const SMSId id);
	
	/**
	 * ”даление несуществующего sms.
	 * Ётот тест кейс не соответствует требовани€м к системе, поскольку в 
	 * реальной работе удалением сообщений (переносом в архив) занимаетс€ 
	 * архиватор.
	 */
	void deleteNonExistentSms(const SMSId id);

	/**
	 * ѕолучение списка sms дл€ повторной доставки.
	 */
	void checkReadyForRetrySms(const vector<SMSId*>& ids,
		const vector<SMS*>& sms, int num);

	/**
	 * ѕолучение списка sms ожидающих доставки на определенный номер.
	 */
	void checkReadyForDeliverySms(const vector<SMSId*>& ids,
		const vector<SMS*>& sms, int num);

	/**
	 * ѕолучение списка sms дл€ выполнени€ операции cancel.
	 */
	void checkReadyForCancelSms(const vector<SMSId*>& ids,
		const vector<SMS*>& sms, int num);

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
	vector<int> checkReadyForDeliverySms(const Address& addr,
		const vector<SMSId*>& ids, const vector<SMS*>& sms);
	vector<int> checkReadyForCancelSms(const Address& oa, 
		const Address& da, const char* svcType, const vector<SMSId*>& ids,
		const vector<SMS*>& sms);
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_TEST_CASES */

