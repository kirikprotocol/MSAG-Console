#ifndef TEST_SME_SMPP_PROTOCOL_ERROR_TEST_CASES
#define TEST_SME_SMPP_PROTOCOL_ERROR_TEST_CASES

#include "test/util/BaseTestCases.hpp"
#include "test/smpp/TestSmppSession.hpp"
#include "SmppFixture.hpp"
#include "smpp/smpp.h"

namespace smsc {
namespace test {
namespace sme {

using log4cpp::Category;
using smsc::sms::Address;
using smsc::sme::SmeConfig;
using smsc::sme::SmppPduEventListener;
using smsc::test::smpp::TestSmppSession;
using smsc::test::util::BaseTestCases;
using smsc::test::util::CheckList;
using smsc::core::synchronization::Event;
using namespace smsc::smpp; //constants, pdu

/**
 * Базовый класс сценариев.
 */
class SmppProtocolErrorScenario : public SmppPduEventListener
{
protected:
	TestSmppSession sess;
	const SmeConfig cfg;
	const Address smeAddr;
	int bindType;
	bool bound;
	Event event;
	bool complete;
	CheckList* chkList;
	
public:
	SmppProtocolErrorScenario(const SmeConfig& conf, const Address& addr,
		CheckList* _chkList)
	: sess(this), cfg(conf), smeAddr(addr), bindType(0), bound(false),
		complete(false), chkList(_chkList) {}
	virtual ~SmppProtocolErrorScenario() {}

	void connect();
	void close();
	void sendPdu(SmppHeader* pdu);

	bool checkComplete(int timeout);
	void setComplete(bool val);
	
	void checkBindResp(SmppHeader* pdu);
	void checkUnbindResp(SmppHeader* pdu);

	SmppHeader* createPdu(uint32_t commandId);
	SmppHeader* setupBindPdu(PduBindTRX& pdu, int bindType);
	SmppHeader* setupUnbindPdu(PduUnbind& pdu);
	SmppHeader* setupSubmitSmPdu(PduSubmitSm& pdu);
	SmppHeader* setupDeliverySmRespPdu(PduDeliverySmResp& pdu, uint32_t seqNum);
	SmppHeader* setupGenericNackPdu(PduGenericNack& pdu, uint32_t seqNum);
	
	virtual void execute() = NULL;
	virtual void handleEvent(SmppHeader* pdu) = NULL;
	virtual void handleError(int errorCode) = NULL;
};

/**
 * Этот класс содержит все test cases необходимые для тестирования ошибочных
 * ситуаций smpp протокола.
 * @author bryz
 */
class SmppProtocolErrorTestCases : public BaseTestCases
{
public:
	SmppProtocolErrorTestCases(const SmeConfig& conf, const Address& addr,
		CheckList* chkList);
	virtual ~SmppProtocolErrorTestCases() {}

	/**
	 * Установливление соединения с SC и отправка неправильной bind pdu.
	 */
	void invalidBindScenario(int num);

	/**
	 * Установливление соединения с SC, правильной bind pdu и отправка
	 * неправильной pdu.
	 */
	void invalidPduScenario(int num);

	/**
	 * Отправка нескольких submit_sm с одинаковыми sequence_number.
	 */
	void equalSequenceNumbersScenario();

	/**
	 * Отправка submit_sm после unbind.
	 */
	void submitAfterUnbindScenario(int num);

	/**
	 * Отправка произвольных pdu с пустыми полями.
	 * Проверка ошибок минимальная, основное назначение - завалить SC.
	 */
	void nullPduScenario(int num);

	/**
	 * Bind sme зарегистрированной в smsc с проверкой внутренностей bind и unbind pdu.
	 */
	void bindUnbindScenario(int num);

	/**
	 * Отправка submit_sm, replace_sm, cancel_sm и т.п. реквестов с sme
	 * зарегистрированной как receiver.
	 */
	void invalidBindStatusScenario(int num);

	/**
	 * Отправка и получение enquire_link pdu.
	 */
	void enquireLinkScenario(int num);

	/**
	 * Если sme не отправляет pdu в течение inactivityTime, то SC
	 * начинает отправлять enquire_link реквесты пока не дождется ответа.
	 */
	void smeInactivityScenario(int num);

protected:
	const SmeConfig cfg;
	const Address smeAddr;
	CheckList* chkList;

	virtual Category& getLog();
};

}
}
}

#endif /* TEST_SME_SMPP_PROTOCOL_ERROR_TEST_CASES */

