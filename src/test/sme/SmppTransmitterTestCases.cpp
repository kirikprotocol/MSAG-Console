#include "SmppTransmitterTestCases.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "util/debug.h"

#define __numTime__ rand1(2)
#define __absoluteTime__ 1

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using namespace smsc::sms; //constants
using namespace smsc::test::smpp; //constants, SmppUtil
using namespace smsc::test::sms; //constants

SmppTransmitterTestCases::SmppTransmitterTestCases(SmppSession* sess,
	const Address& addr, const SmeRegistry* _smeReg,
	SmppResponsePduChecker* _responseChecker)
	: session(sess), smeAddr(addr), smeReg(_smeReg),
	responseChecker(_responseChecker)
{
	__require__(session);
	__require__(smeReg);
	__require__(responseChecker);
	pduReg = smeReg->getPduRegistry(smeAddr); //может быть NULL
}

Category& SmppTransmitterTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppTransmitterTestCases");
	return log;
}

TCResult* SmppTransmitterTestCases::submitSmAssert(int num)
{
	TCSelector s(num, 4);
	TCResult* res = new TCResult(TC_SUBMIT_SM_ASSERT, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			PduSubmitSm pdu;
			switch (s.value())
			{
				case 1: //serviceType больше максимальной длины
					{
						char serviceType[MAX_SERVICE_TYPE_LENGTH + 10];
						rand_char(MAX_SERVICE_TYPE_LENGTH + 1, serviceType);
						pdu.get_message().set_serviceType(serviceType);
					}
					break;
				case 2: //пустой адрес
					{
						PduAddress addr;
						addr.set_typeOfNumber(rand0(255));
						addr.set_numberingPlan(rand0(255));
						addr.set_value("");
						pdu.get_message().set_dest(addr);
					}
					break;
				case 3: //destinationAddress больше максимальной длины
					{
						PduAddress addr;
						addr.set_typeOfNumber(rand0(255));
						addr.set_numberingPlan(rand0(255));
						char addrVal[MAX_ADDRESS_LENGTH + 10];
						rand_char(MAX_ADDRESS_LENGTH + 1, addrVal);
						addr.set_value(addrVal);
						pdu.get_message().set_dest(addr);
					}
					break;
				case 4: //message body больше максимальной длины
					{
						char msg[MAX_MSG_BODY_LENGTH + 10];
						rand_char(MAX_MSG_BODY_LENGTH + 1, msg);
						pdu.get_message().set_shortMessage(msg, MAX_MSG_BODY_LENGTH + 1);
					}
					break;
				default:
					throw s;
			}
			res->addFailure(s.value());
		}
		catch (...)
		{
			//ok
		}
	}
	debug(res);
	return res;
}

TCResult* SmppTransmitterTestCases::submitSmSync(int num)
{
	return submitSm(TC_SUBMIT_SM_SYNC, true, num);
}

TCResult* SmppTransmitterTestCases::submitSmAsync(int num)
{
	return submitSm(TC_SUBMIT_SM_ASYNC, true, num);
}

//метод имеет внутреннюю синхронизацию по pduReg->getMutex()
TCResult* SmppTransmitterTestCases::submitSm(const char* tc, bool sync, int num)
{
	TCSelector s(num, 10);
	TCResult* res = new TCResult(tc, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			PduSubmitSm* pdu = new PduSubmitSm();
			SmppUtil::setupRandomCorrectSubmitSmPdu(pdu);
			PduAddress addr;
			SmppUtil::convert(smeAddr, &addr);
			pdu->get_message().set_source(addr);
			//случайный dest адрес без алиасинга и проверки наличия маршрутов
			PduAddress destAddr;
			const Address* tmp = smeReg->getRandomAddress();
			__require__(tmp);
			SmppUtil::convert(*tmp, &destAddr);
			pdu->get_message().set_dest(destAddr);
			if (pduReg)
			{
				pduReg->getMutex().Lock();
				pdu->get_optional().set_userMessageReference(pduReg->nextMsgRef());
			}
			PduData* replacePduData = NULL;
			switch (s.value())
			{
				case 1: //пустой serviceType
					//pdu->get_message().set_serviceType(NULL);
					pdu->get_message().set_serviceType("");
					break;
				case 2: //serviceType максимальной длины
					{
						EService serviceType;
						rand_char(MAX_ESERVICE_TYPE_LENGTH, serviceType);
						pdu->get_message().set_serviceType(serviceType);
					}
					break;
				case 3: //доставка уже должна была начаться
					{
						SmppTime t;
						SmppUtil::time2string(time(NULL) - rand1(60), t, __absoluteTime__);
						pdu->get_message().set_scheduleDeliveryTime(t);
					}
					break;
				case 4: //срок валидности уже закончился
					{
						SmppTime t;
						SmppUtil::time2string(time(NULL) - rand1(60), t, __absoluteTime__);
						pdu->get_message().set_validityPeriod(t);
					}
					break;
				case 5: //waitTime > validTime
					{
						SmppTime t;
						time_t validTime = time(NULL) + rand0(60);
						time_t waitTime = validTime + rand1(60);
						SmppUtil::time2string(waitTime, t, __numTime__);
						pdu->get_message().set_scheduleDeliveryTime(t);
						SmppUtil::time2string(validTime, t, __numTime__);
						pdu->get_message().set_validityPeriod(t);
					}
					break;
				case 6: //пустое тело сообщения
					pdu->get_message().set_shortMessage(NULL, 0);
					//pdu->get_message().set_shortMessage("", 0);
					break;
				case 7: //тело сообщения максимальной длины
					{
						ShortMessage msg;
						rand_char(MAX_MSG_BODY_LENGTH, msg);
						pdu->get_message().set_shortMessage(msg, MAX_MSG_BODY_LENGTH);
					}
					break;
				case 8: //msgRef одинаковые (эквивалентно msgRef отсутствуют)
					//Согласно GSM 03.40 пункт 9.2.3.25 если совпадают
					//TP-MR, TP-DA, OA, то при ETSI_REJECT_IF_PRESENT будет ошибка.
					//Для SMPP все должно работать независимо от msgRef.
					if (pduReg)
					{
						const PduData* pendingPduData =
							pduReg->getFirstPendingSubmitSmPdu(time(NULL) + rand2(10, 30));
						if (pendingPduData)
						{
							PduSubmitSm* pendingPdu =
								reinterpret_cast<PduSubmitSm*>(pendingPduData->pdu);
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								pendingPdu->get_message().get_dest());
							pdu->get_optional().set_userMessageReference(
								pendingPdu->get_optional().get_userMessageReference());
						}
					}
					break;
				case 9: //отправка дублированного сообщения без замещения уже существующего
					//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
					//destination address and service_type. Сообщение должно быть в 
					//ENROTE state.
					if (pduReg)
					{
						const PduData* pendingPduData =
							pduReg->getFirstPendingSubmitSmPdu(time(NULL) + rand2(10, 30));
						if (pendingPduData)
						{
							PduSubmitSm* pendingPdu =
								reinterpret_cast<PduSubmitSm*>(pendingPduData->pdu);
							pdu->get_message().set_serviceType(
								pendingPdu->get_message().get_serviceType());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								pendingPdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(0);
						}
					}
					break;
				case 10: //отправка дублированного сообщения с замещением уже существующего
					//Согласно SMPP v3.4 пункт 5.2.18 должны совпадать: source address,
					//destination address and service_type. Сообщение должно быть в 
					//ENROTE state.
					if (pduReg)
					{
						replacePduData =
							pduReg->getFirstPendingSubmitSmPdu(time(NULL) + rand2(10, 30));
						if (replacePduData)
						{
							PduSubmitSm* replacePdu =
								reinterpret_cast<PduSubmitSm*>(replacePduData->pdu);
							pdu->get_message().set_serviceType(
								replacePdu->get_message().get_serviceType());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								replacePdu->get_message().get_dest());
							//замещенное сообщение будет доставляться раньше оригинального
							SmppTime tmp;
							time_t waitTime = SmppUtil::string2time(
								replacePdu->get_message().get_scheduleDeliveryTime());
							SmppUtil::time2string(waitTime - rand1(10), tmp, __numTime__);
							pdu->get_message().set_scheduleDeliveryTime(tmp);
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				default:
					throw s;
			}
			//отправить и зарегистрировать pdu
			PduSubmitSmResp* respPdu = NULL;
			if (sync)
			{
				respPdu = session->getSyncTransmitter()->submit(*pdu);
				if (!respPdu)
				{
					res->addFailure(101);
				}
				else
				{
					vector<int> tmp =
						responseChecker->checkSubmitSmResp(*pdu, *respPdu);
					for (int i = 0; i < tmp.size(); i++)
					{
						res->addFailure(110 + tmp[i]);
					}
				}
			}
			else
			{
				if (session->getAsyncTransmitter()->submit(*pdu))
				{
					res->addFailure(201);
				}
			}
			if (pduReg)
			{
				//Зарегистрировать pdu
				PduData pduData(reinterpret_cast<SmppHeader*>(pdu));
				if (respPdu)
				{
					pduData.smsId = SmppUtil::convert(respPdu->get_messageId());
					pduData.responseFlag = true;
				}
				pduData.msgRef = pdu->get_optional().get_userMessageReference();
				pduData.waitTime =
					SmppUtil::convert(pdu->get_message().get_scheduleDeliveryTime());
				//pduData.responseFlag = false;
				pduData.deliveryFlag = false;
				//если delivery receipt и intermediate notifications не должно быть,
				//помечаем их как уже полученные 
				pduData.deliveryReceiptFlag =
					pdu->get_message().get_registredDelivery() &
					SMSC_DELIVERY_RECEIPT_BITS == NO_SMSC_DELIVERY_RECEIPT;
				pduData.intermediateNotificationFlag =
					pdu->get_message().get_registredDelivery() &
					INTERMEDIATE_NOTIFICATION_REQUESTED;
				pduData.replacePdu = replacePduData;
				pduReg->putPdu(pduData);
				pduReg->getMutex().Unlock();
				//pdu life time определяется PduRegistry
				//delete pdu;
			}
			else
			{
				//если ничего проверять не надо (для performance testing)
				delete pdu;
			}
			delete respPdu;
		}
		catch(...)
		{
			error();
			res->addFailure(s.value());
		}
	}
	debug(res);
	return res;
}

}
}
}

