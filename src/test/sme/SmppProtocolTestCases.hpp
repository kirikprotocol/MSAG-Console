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
using smsc::test::util::TestCaseId;

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
	 * Заполнение и отправка submit_sm pdu с недопустимыми значениями полей.
	 */
	void submitSmAssert(int num);

	/**
	 * Отправка корректной data_sm pdu другим sme.
	 */
	void dataSmCorrect(bool sync, int num);

	/**
	 * Отправка некорректной data_sm pdu другим sme.
	 */
	void dataSmIncorrect(bool sync, int num);

	/**
	 * Заполнение и отправка submit_sm или data_sm pdu с правильными директивами.
	 */
	void correctDirectives(bool sync, const TestCaseId& num);

	/**
	 * Заполнение и отправка submit_sm или data_sm pdu с неправильными директивами.
	 */
	void incorrectDirectives(bool sync, const TestCaseId& num);

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
	PduData* getNonReplaceEnrotePdu();
	PduData* getReplaceEnrotePdu();
	PduData* getNonReplaceRescheduledEnrotePdu();
	PduData* getFinalPdu();
	PduData* getPduByState(SmppState state);

	bool setDirective(SmppHeader* header, const string& dir, int& offset);
	bool correctAckDirectives(SmppHeader* header, PduData::IntProps& intProps, int num);
	bool correctDefDirectives(SmppHeader* header, PduData::IntProps& intProps, int num);
	bool correctTemplateDirectives(SmppHeader* header, PduData::IntProps& intProps,
		PduData::StrProps& strProps, int num);
	bool correctHideDirectives(SmppHeader* header, PduData::IntProps& intProps, int num);
	void correctDirectives(SmppHeader* header, PduData::IntProps& intProps,
		PduData::StrProps& strProps, const TestCaseId& num);
	void incorrectDirectives(SmppHeader* header, PduData::IntProps& intProps, int num);

	void replaceSmIncorrect(PduReplaceSm* pdu, bool sync);
	PduData* getCancelSmGroupParams(bool checkServType, Address& srcAddr,
		Address& destAddr, string& servType);
	int getRandomRespDelay();
};

}
}
}

#endif /* TEST_SME_SMPP_PROTOCOL_TEST_CASES */
