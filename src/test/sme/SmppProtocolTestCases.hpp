#ifndef TEST_SME_SMPP_PROTOCOL_TEST_CASES
#define TEST_SME_SMPP_PROTOCOL_TEST_CASES

#include "test/util/BaseTestCases.hpp"
#include "SmppFixture.hpp"
#include "SmppTransmitterTestCases.hpp"

namespace smsc {
namespace test {
namespace sme {

using log4cpp::Category;
using smsc::test::util::BaseTestCases;
using smsc::test::util::CheckList;

/**
 * Этот класс содержит все test cases необходимые для тестирования sme.
 * @author bryz
 */
class SmppProtocolTestCases : public BaseTestCases
{
public:
	SmppProtocolTestCases(SmppFixture* _fixture)
	: fixture(_fixture), chkList(_fixture->chkList) {}
	
	virtual ~SmppProtocolTestCases() {}

	/**
	 * Отправка корректной submit_sm pdu другим sme.
	 */
	void submitSmCorrect(bool sync, int num);

	/**
	 * Отправка некорректной submit_sm pdu другим sme.
	 */
	void submitSmIncorrect(bool sync, int num);

	/**
	 * Заполнение и отправка submit_sm pdu с недопустимыми значениями полей.
	 */
	void submitSmAssert(int num);

	/**
	 * Замещение ранее отправленной submit_sm pdu.
	 */
	void replaceSm(bool sync, int num);

	/**
	 * Заполнение и отправка replace_sm pdu с недопустимыми значениями полей.
	 */
	void replaceSmAssert(int num);
	
	/**
	 * Отправка синхронного или асинхронного deliver_sm_resp со статусом ok.
	 */
	uint32_t sendDeliverySmRespOk(PduDeliverySm& pdu, bool sync);

	/**
	 * Отправка синхронного или асинхронного deliver_sm_resp с кодом ошибки
	 * и последующей повторной доставкой.
	 */
	uint32_t sendDeliverySmRespRetry(PduDeliverySm& pdu, bool sync, int num);

	/**
	 * Отправка синхронного или асинхронного deliver_sm_resp с кодом ошибки
	 * и последующем прекращением доставки.
	 */
	uint32_t sendDeliverySmRespError(PduDeliverySm& pdu, bool sync, int num);

protected:
	SmppFixture* fixture;
	CheckList* chkList;
	
	virtual Category& getLog();
	PduData* getNonReplaceEnrotePdu();
	PduData* getReplaceEnrotePdu();
	PduData* getNonReplaceRescheduledEnrotePdu();
	PduData* getFinalPdu();
};

}
}
}

#endif /* TEST_SME_SMPP_PROTOCOL_TEST_CASES */
