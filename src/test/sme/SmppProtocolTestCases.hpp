#ifndef TEST_SME_SMPP_PROTOCOL_TEST_CASES
#define TEST_SME_SMPP_PROTOCOL_TEST_CASES

#include "test/util/BaseTestCases.hpp"
#include "SmppFixture.hpp"
#include "SmppTransmitterTestCases.hpp"

namespace smsc {
namespace test {
namespace sme {

using log4cpp::Category;
using smsc::smeman::SmeInfo;
using smsc::test::util::BaseTestCases;
using smsc::test::util::CheckList;

/**
 * Этот класс содержит все test cases необходимые для тестирования sme.
 * @author bryz
 */
class SmppProtocolTestCases : public BaseTestCases
{
public:
	SmppProtocolTestCases(SmppFixture* fixture);
	virtual ~SmppProtocolTestCases() {}

	/**
	 * Отправка корректной submit_sm pdu другим sme.
	 */
	void submitSmCorrect(bool sync, int num);

	/**
	 * Отправка корректной "сложной" submit_sm pdu другим sme.
	 */
	void submitSmCorrectComplex(bool sync, int num);

	/**
	 * Отправка некорректной submit_sm pdu другим sme.
	 */
	void submitSmIncorrect(bool sync, int num);

	/**
	 * Отправка корректной data_sm pdu другим sme.
	 */
	void dataSmCorrect(bool sync, int num);

	/**
	 * Отправка некорректной data_sm pdu другим sme.
	 */
	void dataSmIncorrect(bool sync, int num);

	/**
	 * Заполнение и отправка submit_sm pdu с недопустимыми значениями полей.
	 */
	void submitSmAssert(int num);

	/**
	 * Корректное замещение ранее отправленной submit_sm pdu.
	 */
	void replaceSmCorrect(bool sync, int num);

	/**
	 * Некорректное замещение ранее отправленной submit_sm pdu.
	 */
	void replaceSmIncorrect(bool sync, int num);

	/**
	 * Заполнение и отправка replace_sm pdu с недопустимыми значениями полей.
	 */
	void replaceSmAssert(int num);

	/**
	 * Запрос статуса существующего sms.
	 */
	void querySmCorrect(bool sync, int num);

	/**
	 * Запрос статуса несуществующего sms.
	 */
	void querySmIncorrect(bool sync, int num);

	/**
	 * Отмена одиночной существующей sms.
	 */
	void cancelSmSingleCorrect(bool sync, int num);

	/**
	 * Отмена группы существующих sms.
	 */
	void cancelSmGroupCorrect(bool sync, int num);

	/**
	 * Отмена несуществующего sms.
	 */
	void cancelSmIncorrect(bool sync, int num);

	/**
	 * Отправка ошибки.
	 */
	void genericNack(bool sync);

	/**
	 * Отправка синхронного или асинхронного deliver_sm_resp со статусом ok.
	 */
	pair<uint32_t, time_t> sendDeliverySmRespOk(SmppHeader* header,
		bool sync, bool sendDelay);

	/**
	 * Отправка синхронного или асинхронного deliver_sm_resp с кодом ошибки
	 * и последующей повторной доставкой.
	 */
	pair<uint32_t, time_t> sendDeliverySmRespRetry(SmppHeader* header,
		bool sync, int num);

	/**
	 * Отправка синхронного или асинхронного deliver_sm_resp с кодом ошибки
	 * и последующем прекращением доставки.
	 */
	pair<uint32_t, time_t> sendDeliverySmRespError(SmppHeader* header,
		bool sync, bool sendDelay, int num);

	/**
	 * Отправка синхронного или асинхронного data_sm_resp со статусом ok.
	 */
	pair<uint32_t, time_t> sendDataSmRespOk(SmppHeader* header,
		bool sync, bool sendDelay);

	/**
	 * Отправка синхронного или асинхронного data_sm_resp с кодом ошибки
	 * и последующей повторной доставкой.
	 */
	pair<uint32_t, time_t> sendDataSmRespRetry(SmppHeader* header,
		bool sync, int num);

	/**
	 * Отправка синхронного или асинхронного data_sm_resp с кодом ошибки
	 * и последующем прекращением доставки.
	 */
	pair<uint32_t, time_t> sendDataSmRespError(SmppHeader* header,
		bool sync, bool sendDelay, int num);

	void sendInvalidPdu(bool sync, int num);

protected:
	SmppFixture* fixture;
	CheckList* chkList;

	virtual Category& getLog();
	PduData* getNonReplaceEnrotePdu(bool deliveryReports);
	PduData* getReplaceEnrotePdu(bool deliveryReports);
	PduData* getNonReplaceRescheduledEnrotePdu(bool deliveryReports);
	PduData* getFinalPdu(bool deliveryReports);
	PduData* getPduByState(SmppState state);
	void replaceSmIncorrect(PduReplaceSm* pdu, bool sync);
	PduData* getCancelSmGroupParams(bool checkServType, Address& srcAddr,
		Address& destAddr, string& servType);
	int getRandomRespDelay();
};

}
}
}

#endif /* TEST_SME_SMPP_PROTOCOL_TEST_CASES */
