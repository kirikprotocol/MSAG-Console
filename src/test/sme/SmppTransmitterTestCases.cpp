#include "SmppTransmitterTestCases.hpp"
#include "test/TestConfig.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using namespace smsc::sms; //constants
using namespace smsc::test; //config constants
using namespace smsc::test::smpp; //constants, SmppUtil
using namespace smsc::test::sms; //constants
using namespace smsc::smpp;
using namespace smsc::smpp::SmppCommandSet;

SmppTransmitterTestCases::SmppTransmitterTestCases(SmppSession* sess,
	const SmeSystemId& id, const Address& addr, const SmeRegistry* _smeReg,
	RouteChecker* _routeChecker, SmppPduChecker* _pduChecker, CheckList* _chkList)
	: session(sess), systemId(id), smeAddr(addr), smeReg(_smeReg),
	routeChecker(_routeChecker), pduChecker(_pduChecker), chkList(_chkList)
{
	__require__(session);
	__require__(smeReg);
	//__require__(routeChecker);
	//__require__(pduChecker);
	//__require__(chkList);
	pduReg = smeReg->getPduRegistry(smeAddr); //����� ���� NULL
}

Category& SmppTransmitterTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppTransmitterTestCases");
	return log;
}

void SmppTransmitterTestCases::submitSmAssert(int num)
{
	TCSelector s(num, 4);
	__decl_tc__;
	for (; s.check(); s++)
	{
		try
		{
			PduSubmitSm pdu;
			switch (s.value())
			{
				case 1: //serviceType ������ ������������ �����
					{
						__tc__("submitSmAssert.serviceTypeInvalid");
						char serviceType[MAX_SERVICE_TYPE_LENGTH + 10];
						rand_char(MAX_SERVICE_TYPE_LENGTH + 1, serviceType);
						pdu.get_message().set_serviceType(serviceType);
					}
					break;
				case 2: //������ ����� �����������
					{
						__tc__("submitSmAssert.destAddrLengthInvalid");
						PduAddress addr;
						addr.set_typeOfNumber(rand0(255));
						addr.set_numberingPlan(rand0(255));
						addr.set_value("");
						pdu.get_message().set_dest(addr);
					}
					break;
				case 3: //destinationAddress ������ ������������ �����
					{
						__tc__("submitSmAssert.destAddrLengthInvalid");
						PduAddress addr;
						addr.set_typeOfNumber(rand0(255));
						addr.set_numberingPlan(rand0(255));
						char addrVal[MAX_ADDRESS_LENGTH + 10];
						rand_char(MAX_ADDRESS_LENGTH + 1, addrVal);
						addr.set_value(addrVal);
						pdu.get_message().set_dest(addr);
					}
					break;
				case 4: //message body ������ ������������ �����
					{
						__tc__("submitSmAssert.msgLengthInvalid");
						int len = MAX_SM_LENGTH + 1;
						auto_ptr<char> msg = rand_char(len);
						pdu.get_message().set_shortMessage(msg.get(), len);
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			__dumpSubmitSmPdu__("SmppTransmitterTestCases::submitSmAssert", systemId, &pdu);
			__tc_fail__(s.value());
		}
		catch (...)
		{
			__tc_ok__;
		}
	}
}

PduData* SmppTransmitterTestCases::getNonReplaceEnrotePdu()
{
	__require__(pduReg);
	PduRegistry::PduDataIterator* it = pduReg->getPduByWaitTime(
		time(NULL) + sequentialPduInterval, LONG_MAX);
	//��� ���������� ������������ � ������������ ����� pdu
	PduData* pduData = NULL;
	while (PduData* data = it->next())
	{
		if (!data->replacedByPdu && !data->replacePdu &&
			data->smsId.length() && data->deliveryFlag == PDU_REQUIRED_FLAG)
		{
			pduData = data;
			break;
		}
	}
	delete it;
	return pduData;
}

PduData* SmppTransmitterTestCases::getReplaceEnrotePdu()
{
	__require__(pduReg);
	PduRegistry::PduDataIterator* it = pduReg->getPduByWaitTime(
		time(NULL) + sequentialPduInterval, LONG_MAX);
	//��� ���������� � ������������ ����� pdu
	PduData* pduData = NULL;
	while (PduData* data = it->next())
	{
		if (!data->replacedByPdu && data->replacePdu &&
			data->smsId.length() && data->deliveryFlag == PDU_REQUIRED_FLAG)
		{
			pduData = data;
			break;
		}
	}
	delete it;
	return pduData;
}

PduData* SmppTransmitterTestCases::getNonReplaceRescheduledEnrotePdu()
{
	__require__(pduReg);
	PduRegistry::PduDataIterator* it = pduReg->getPduByWaitTime(0, time(NULL));
	//��� ������������ ����� pdu
	PduData* pduData = NULL;
	while (PduData* data = it->next())
	{
		if (!data->replacedByPdu && data->smsId.length() &&
			data->deliveryFlag == PDU_REQUIRED_FLAG &&
			data->deliveryFlag.getNextTime(time(NULL)) >=
			time(NULL) + sequentialPduInterval)
		{
			pduData = data;
			break;
		}
	}
	delete it;
	return pduData;
}

template <class Message>
void SmppTransmitterTestCases::checkRegisteredDelivery(Message& m)
{
	uint8_t regDelivery = m.get_registredDelivery();
	if ((regDelivery & SMSC_DELIVERY_RECEIPT_BITS) == FINAL_SMSC_DELIVERY_RECEIPT ||
		(regDelivery & SMSC_DELIVERY_RECEIPT_BITS) == FAILURE_SMSC_DELIVERY_RECEIPT ||
		(regDelivery & INTERMEDIATE_NOTIFICATION_REQUESTED))
	{
		SmppTime t;
		time_t waitTime = time(NULL) + rand2(sequentialPduInterval, 60);
		time_t validTime = waitTime + rand1(60);
		m.set_scheduleDeliveryTime(
			SmppUtil::time2string(waitTime, t, time(NULL), __numTime__));
		m.set_validityPeriod(
			SmppUtil::time2string(validTime, t, time(NULL), __numTime__));
	}
}

//��������������� ����������� pdu, ��������� ������� �������������
PduData* SmppTransmitterTestCases::registerSubmitSm(PduSubmitSm* pdu,
	PduData* replacePduData)
{
	__require__(pduReg);
	PduData* pduData = new PduData(pdu->get_optional().get_userMessageReference(),
		time(NULL),
		max(time(NULL), SmppUtil::string2time(pdu->get_message().get_scheduleDeliveryTime(), time(NULL))),
		SmppUtil::string2time(pdu->get_message().get_validityPeriod(), time(NULL)),
		reinterpret_cast<SmppHeader*>(pdu));
	//��� ������ ����� ������� ��������, ��������� �������� �
	//checkSubmitSmResp, checkSubmitTime, checkWaitTime, checkValidTime
	//pduData->responseFlag = PDU_REQUIRED_FLAG;
	pduData->deliveryFlag = routeChecker->isDestReachable(
		pdu->get_message().get_dest(), false) ?
		PDU_REQUIRED_FLAG : PDU_NOT_EXPECTED_FLAG;
	uint8_t regDelivery = pdu->get_message().get_registredDelivery();
	pduData->deliveryReceiptFlag =
		((regDelivery & SMSC_DELIVERY_RECEIPT_BITS) == NO_SMSC_DELIVERY_RECEIPT ?
		PDU_NOT_EXPECTED_FLAG : PDU_REQUIRED_FLAG);
	pduData->intermediateNotificationFlag =
		((regDelivery & INTERMEDIATE_NOTIFICATION_REQUESTED) ?
		PDU_REQUIRED_FLAG : PDU_NOT_EXPECTED_FLAG);
	pduData->replacePdu = replacePduData;
	if (replacePduData)
	{
		replacePduData->replacedByPdu = pduData;
	}
	pduReg->registerPdu(pduData);
	return pduData;
}

//�������� smsId, sequenceNumber � PduRegistry � ��������� pdu
//��������� ������� �������������
void SmppTransmitterTestCases::processSubmitSmSync(PduData* pduData,
	PduSubmitSmResp* respPdu)
{
	__require__(pduReg);
	__require__(pduData);
	__dumpPdu__("processSubmitSmRespSync", systemId, respPdu);
	__decl_tc__;
	__tc__("processSubmitSmResp.sync");
	if (!respPdu)
	{
		__tc_fail__(1);
		pduData->responseFlag = PDU_MISSING_ON_TIME_FLAG;
		//�������� sequenceNumber
		pduReg->updatePdu(pduData);
	}
	else
	{
		pduChecker->processSubmitSmResp(pduData, *respPdu, time(NULL));
		delete respPdu; //disposePdu
	}
	__tc_ok_cond__;
	//pduReg->updatePdu(pduData);  //��� ��������� � processReplaceSmResp()
}

//�������� sequenceNumber � PduRegistry, ��������� ������� �������������
void SmppTransmitterTestCases::processSubmitSmAsync(PduData* pduData,
	PduSubmitSmResp* respPdu)
{
	__require__(pduReg);
	__require__(pduData);
	__decl_tc__;
	__tc__("processSubmitSmResp.async");
	if (respPdu)
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
}

void SmppTransmitterTestCases::submitSm(bool sync, int num)
{
	TCSelector s(num, 15);
	__decl_tc12__;
	__tc1__("submitSm");
	if (sync)
	{
		__tc2__("submitSm.sync");
	}
	else
	{
		__tc2__("submitSm.async");
	}
	for (; s.check(); s++)
	{
		try
		{
			PduSubmitSm* pdu = new PduSubmitSm();
			SmppUtil::setupRandomCorrectSubmitSmPdu(pdu);
			 //Default message Type (i.e. normal message)
			pdu->get_message().set_esmClass(
				pdu->get_message().get_esmClass() & 0xc3);
			//���� ��������� ������������� �������� ��� ������������� �����������,
			//���������� ���������� ��������
			checkRegisteredDelivery(pdu->get_message());
			//source
			PduAddress srcAddr;
			SmppUtil::convert(smeAddr, &srcAddr);
			pdu->get_message().set_source(srcAddr);
			//��������� dest ����� ��� �������� ������� ���������
			PduAddress destAddr;
			const Address* tmp = smeReg->getRandomAddress();
			__require__(tmp);
			SmppUtil::convert(*tmp, &destAddr);
			pdu->get_message().set_dest(destAddr);
			//msgRef
			if (pduReg)
			{
				pdu->get_optional().set_userMessageReference(pduReg->nextMsgRef());
			}
			PduData* replacePduData = NULL;
			switch (s.value())
			{
				case 1: //������ ����������
					//__tc1__("submitSm");
					break;
				/*
				case 2: //������ serviceType
					__tc1__("submitSm.serviceTypeMarginal");
					//pdu->get_message().set_serviceType(NULL);
					pdu->get_message().set_serviceType("");
					break;
				*/
				case 2: //serviceType ������������ �����
					{
						__tc1__("submitSm.serviceTypeMarginal");
						EService serviceType;
						rand_char(MAX_SERVICE_TYPE_LENGTH, serviceType);
						pdu->get_message().set_serviceType(serviceType);
					}
					break;
				case 3: //�������� ��� ������ ���� ��������
					{
						__tc1__("submitSm.waitTimePast");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) - rand1(60), t, time(NULL), __absoluteTime__);
						pdu->get_message().set_scheduleDeliveryTime(t);
						//�������� ������������� �������� � �����������
						pdu->get_message().set_registredDelivery(0);
					}
					break;
				case 4: //���� ���������� ��� ����������
					{
						__tc1__("submitSm.validTimePast");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) - rand1(60), t, time(NULL), __absoluteTime__);
						pdu->get_message().set_validityPeriod(t);
					}
					break;
				case 5: //���� ���������� ������ �������������
					{
						__tc1__("submitSm.validTimeExceeded");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) + maxValidPeriod + timeCheckAccuracy, t,
							time(NULL), __numTime__);
						pdu->get_message().set_validityPeriod(t);
					}
					break;
				case 6: //waitTime > validTime
					{
						__tc1__("submitSm.waitTimeInvalid");
						SmppTime t;
						time_t validTime = time(NULL) + rand1(60);
						time_t waitTime = validTime + rand1(60);
						SmppUtil::time2string(waitTime, t, time(NULL), __numTime__);
						pdu->get_message().set_scheduleDeliveryTime(t);
						SmppUtil::time2string(validTime, t, time(NULL), __numTime__);
						pdu->get_message().set_validityPeriod(t);
					}
					break;
				case 7: //������ ���� ���������
					__tc1__("submitSm.smLengthMarginal");
					pdu->get_message().set_shortMessage(NULL, 0);
					//pdu->get_message().set_shortMessage("", 0);
					break;
				case 8: //���� ��������� ������������ �����
					{
						__tc1__("submitSm.smLengthMarginal");
						ShortMessage msg;
						rand_char(MAX_SM_LENGTH, msg);
						pdu->get_message().set_shortMessage(msg, MAX_SM_LENGTH);
					}
					break;
				case 9: //msgRef ���������� (������������ msgRef �����������)
					//�������� GSM 03.40 ����� 9.2.3.25 ���� ���������
					//TP-MR, TP-DA, OA, �� ��� ETSI_REJECT_IF_PRESENT ����� ������.
					//��� SMPP ��� ������ �������� ���������� �� msgRef.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
						PduData* pendingPduData = getNonReplaceEnrotePdu();
						if (pendingPduData)
						{
							__require__(pendingPduData->pdu &&
								pendingPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc1__("submitSm.checkMap");
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
				case 10: //�������� �������������� ��������� ��� ��������� ��� �������������
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
						PduData* pendingPduData = getNonReplaceEnrotePdu();
						if (pendingPduData)
						{
							__require__(pendingPduData->pdu &&
								pendingPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc1__("submitSm.notReplace");
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
				case 11: //�������� �������������� ��������� � �������� ���������
					//��� �������������, �� service_type �� ���������.
					//��� ������������� source address � destination address
					//���� ������ �� �����, �.�. ��������� �������� � ����������.
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
						PduData* pendingPduData = getNonReplaceEnrotePdu();
						if (pendingPduData)
						{
							__require__(pendingPduData->pdu &&
								pendingPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc1__("submitSm.serviceTypeNotMatch");
							PduSubmitSm* pendingPdu =
								reinterpret_cast<PduSubmitSm*>(pendingPduData->pdu);
							auto_ptr<char> serviceType = rand_char(MAX_SERVICE_TYPE_LENGTH);
							pdu->get_message().set_serviceType(serviceType.get());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								pendingPdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				case 12: //�������� �������������� ��������� � ���������� ��� �������������
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
						replacePduData = getNonReplaceEnrotePdu();
						if (replacePduData)
						{
							__require__(replacePduData->pdu &&
								replacePduData->pdu->get_commandId() == SUBMIT_SM);
							__tc1__("submitSm.replaceEnrote");
							PduSubmitSm* replacePdu =
								reinterpret_cast<PduSubmitSm*>(replacePduData->pdu);
							pdu->get_message().set_serviceType(
								replacePdu->get_message().get_serviceType());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								replacePdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				case 13: //�������� �������������� ��������� � ���������� ���
					//����� �����������
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
						replacePduData = getReplaceEnrotePdu();
						if (replacePduData)
						{
							__require__(replacePduData->pdu &&
								replacePduData->pdu->get_commandId() == SUBMIT_SM);
							__tc1__("submitSm.replaceReplacedEnrote");
							PduSubmitSm* replacePdu =
								reinterpret_cast<PduSubmitSm*>(replacePduData->pdu);
							pdu->get_message().set_serviceType(
								replacePdu->get_message().get_serviceType());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								replacePdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				case 14: //�������� �������������� ��������� � ���������� ���
					//�������������, �� ������������ ��� � ��������� ���������.
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
						PduData* finalPduData = pduReg->getLastRemovedPdu();
						if (finalPduData)
						{
							__require__(finalPduData->pdu &&
								finalPduData->pdu->get_commandId() == SUBMIT_SM);
							__tc1__("submitSm.replaceFinal");
							PduSubmitSm* finalPdu =
								reinterpret_cast<PduSubmitSm*>(finalPduData->pdu);
							pdu->get_message().set_serviceType(
								finalPdu->get_message().get_serviceType());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								finalPdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				case 15: //�������� �������������� ��������� � ���������� ���
					//�������������, �� ������������ ��� � �������� ��������� ��������.
					//�������� SMPP v3.4 ����� 5.2.18 ������ ���������: source address,
					//destination address and service_type. ��������� ������ ���� � 
					//ENROTE state.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
						replacePduData = getNonReplaceRescheduledEnrotePdu();
						if (replacePduData)
						{
							__require__(replacePduData->pdu &&
								replacePduData->pdu->get_commandId() == SUBMIT_SM);
							__tc1__("submitSm.replaceRepeatedDeliveryEnrote");
							PduSubmitSm* replacePdu =
								reinterpret_cast<PduSubmitSm*>(replacePduData->pdu);
							pdu->get_message().set_serviceType(
								replacePdu->get_message().get_serviceType());
							//pdu->get_message().set_source(...);
							pdu->get_message().set_dest(
								replacePdu->get_message().get_dest());
							pdu->get_message().set_replaceIfPresentFlag(1);
						}
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			//��������� � ���������������� pdu
			if (pduReg)
			{
				if (sync)
				{
					PduData* pduData;
					{
						MutexGuard mguard(pduReg->getMutex());
						pdu->get_header().set_sequenceNumber(0); //�� ��������
						pduData = registerSubmitSm(pdu, replacePduData); //all times, msgRef
					}
					__dumpSubmitSmPdu__("submitSmSyncBefore", systemId, pdu);
					PduSubmitSmResp* respPdu = session->getSyncTransmitter()->submit(*pdu);
					__dumpSubmitSmPdu__("submitSmSyncAfter", systemId, pdu);
					{
						MutexGuard mguard(pduReg->getMutex());
						processSubmitSmSync(pduData, respPdu); //smsId, sequenceNumber
					}
				}
				else
				{
					MutexGuard mguard(pduReg->getMutex());
					__dumpSubmitSmPdu__("submitSmAsyncBefore", systemId, pdu);
					PduSubmitSmResp* respPdu = session->getAsyncTransmitter()->submit(*pdu);
					__dumpSubmitSmPdu__("submitSmAsyncAfter", systemId, pdu);
					PduData* pduData = registerSubmitSm(pdu, replacePduData); //all times, msgRef, sequenceNumber
					processSubmitSmAsync(pduData, respPdu);
				}
				//pdu life time ������������ PduRegistry
				//disposePdu(pdu);
			}
			else
			{
				if (sync)
				{
					PduSubmitSmResp* respPdu =
						session->getSyncTransmitter()->submit(*pdu);
					if (respPdu)
					{
						delete respPdu; //disposePdu
					}
				}
				else
				{
					session->getAsyncTransmitter()->submit(*pdu);
				}
				delete pdu; //disposePdu
			}
			__tc12_ok_cond__;
		}
		catch(...)
		{
			__tc12_fail__(s.value());
			error();
		}
	}
}

//��������������� ����������� pdu, ��������� ������� �������������
PduData* SmppTransmitterTestCases::registerReplaceSm(PduReplaceSm* pdu,
	PduData* replacePduData)
{
	if (replacePduData)
	{
		__require__(replacePduData->pdu &&
			replacePduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* replacePdu = reinterpret_cast<PduSubmitSm*>(replacePduData->pdu);
		PduSubmitSm* resPdu = new PduSubmitSm(*replacePdu);
		resPdu->get_message().set_scheduleDeliveryTime(pdu->get_scheduleDeliveryTime());
		resPdu->get_message().set_validityPeriod(pdu->get_validityPeriod());
		resPdu->get_message().set_registredDelivery(pdu->get_registredDelivery());
		resPdu->get_message().set_smDefaultMsgId(pdu->get_smDefaultMsgId());
		resPdu->get_message().set_shortMessage(pdu->get_shortMessage(), pdu->size_shortMessage());
		PduData* pduData = registerSubmitSm(resPdu, replacePduData);
		pduData->smsId = replacePduData->smsId;
		return pduData;
	}
	else
	{
		__require__(pduReg);
		PduData* pduData = new PduData(0, 0, 0, 0, reinterpret_cast<SmppHeader*>(pdu));
		pdu->get_header().set_sequenceNumber(0); //�� ��������
		//pduData->responseFlag = PDU_REQUIRED_FLAG;
		pduData->deliveryFlag = PDU_NOT_EXPECTED_FLAG;
		pduData->deliveryReceiptFlag = PDU_NOT_EXPECTED_FLAG;
		pduData->intermediateNotificationFlag = PDU_NOT_EXPECTED_FLAG;
		//pduReg->registerPdu(pduData);
		return pduData;
	}
}

//�������� sequenceNumber � PduRegistry � ��������� pdu
//��������� ������� �������������
void SmppTransmitterTestCases::processReplaceSmSync(PduData* pduData,
	PduReplaceSmResp* respPdu)
{
	__require__(pduData);
	__dumpPdu__("processReplaceSmRespSync", systemId, respPdu);
	__decl_tc__;
	__tc__("processReplaceSmResp.sync");
	if (!respPdu)
	{
		__tc_fail__(1);
		pduData->responseFlag = PDU_MISSING_ON_TIME_FLAG;
	}
	else
	{
		pduChecker->processReplaceSmResp(pduData, *respPdu, time(NULL));
		delete respPdu; //disposePdu
	}
	__tc_ok_cond__;
	//pduReg->updatePdu(pduData); //��� ��������� � processReplaceSmResp()
}

//�������� sequenceNumber � PduRegistry, ��������� ������� �������������
void SmppTransmitterTestCases::processReplaceSmAsync(PduData* pduData,
	PduReplaceSmResp* respPdu)
{
	__require__(pduData);
	__decl_tc__;
	__tc__("processReplaceSmResp.async");
	if (respPdu)
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
}

void SmppTransmitterTestCases::replaceSm(bool sync, int num)
{
	TCSelector s(num, 11);
	__decl_tc12__;
	__tc1__("replaceSm");
	__tc2__(sync ? "replaceSm.sync" : "replaceSm.async");
	for (; s.check(); s++)
	{
		try
		{
			PduReplaceSm* pdu = new PduReplaceSm();
			SmppUtil::setupRandomCorrectReplaceSmPdu(pdu);
			//���� ��������� ������������� �������� ��� ������������� �����������,
			//���������� ���������� ��������
			checkRegisteredDelivery(*pdu);
			//source
			PduAddress srcAddr;
			SmppUtil::convert(smeAddr, &srcAddr);
			pdu->set_source(srcAddr);
			//������� ��������� messageId �� sms ��������� ��������
			PduData* replacePduData = NULL;
			if (pduReg)
			{
				MutexGuard mguard(pduReg->getMutex());
				replacePduData = getNonReplaceEnrotePdu();
			}
			//������ �������� �������������� messageId
			if (replacePduData)
			{
				pdu->set_messageId(replacePduData->smsId.c_str());
			}
			else
			{
				auto_ptr<char> msgId = rand_char(MAX_MSG_ID_LENGTH);
				pdu->set_messageId(msgId.get());
			}
			switch (s.value())
			{
				case 1: //������ ����������
					//__tc1__("replaceSm");
					break;
				case 2: //�������� ��� ������ ���� ��������
					{
						__tc1__("replaceSm.waitTimePast");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) - rand1(60), t, time(NULL), __absoluteTime__);
						pdu->set_scheduleDeliveryTime(t);
						//�������� ������������� �������� � �����������
						pdu->set_registredDelivery(0);
					}
					break;
				case 3: //���� ���������� ��� ����������
					{
						__tc1__("replaceSm.validTimePast");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) - rand1(60), t, time(NULL), __absoluteTime__);
						pdu->set_validityPeriod(t);
					}
					break;
				case 4: //���� ���������� ������ �������������
					{
						__tc1__("replaceSm.validTimeExceeded");
						SmppTime t;
						SmppUtil::time2string(
							time(NULL) + maxValidPeriod + timeCheckAccuracy, t,
							time(NULL), __numTime__);
						pdu->set_validityPeriod(t);
					}
					break;
				case 5: //waitTime > validTime
					{
						__tc1__("replaceSm.waitTimeInvalid");
						SmppTime t;
						time_t validTime = time(NULL) + rand1(60);
						time_t waitTime = validTime + rand1(60);
						SmppUtil::time2string(waitTime, t, time(NULL), __numTime__);
						pdu->set_scheduleDeliveryTime(t);
						SmppUtil::time2string(validTime, t, time(NULL), __numTime__);
						pdu->set_validityPeriod(t);
					}
					break;
				case 6: //������ ���� ���������
					__tc1__("replaceSm.smLengthMarginal");
					pdu->set_shortMessage(NULL, 0);
					//pdu->set_shortMessage("", 0);
					break;
				case 7: //���� ��������� ������������ �����
					{
						__tc1__("replaceSm.smLengthMarginal");
						auto_ptr<char> msg = rand_char(MAX_SM_LENGTH);
						pdu->set_shortMessage(msg.get(), MAX_SM_LENGTH);
					}
					break;
				case 8: //����������� ��������� �� ����������
					{
						__tc1__("replaceSm.msgIdNotExist");
						auto_ptr<char> msgId = rand_char(MAX_MSG_ID_LENGTH);
						pdu->set_messageId(msgId.get());
						replacePduData = NULL;
					}
					break;
				case 9: //��������� ��� ����� ����������� ���������
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
						replacePduData = getReplaceEnrotePdu();
						if (replacePduData)
						{
							__tc1__("replaceSm.replaceReplacedEnrote");
							pdu->set_messageId(replacePduData->smsId.c_str());
						}
					}
					break;
				case 10: //���������� ������������� ���������, �� ������������
					//��� � ��������� ���������.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
						PduData* finalPduData = pduReg->getLastRemovedPdu();
						if (finalPduData)
						{
							__tc1__("replaceSm.replaceFinal");
							pdu->set_messageId(finalPduData->smsId.c_str());
						}
					}
					break;
				case 11: //��������� ������������� ���������, �� ������������
					//��� � �������� ��������� ��������.
					if (pduReg)
					{
						MutexGuard mguard(pduReg->getMutex());
						replacePduData = getNonReplaceRescheduledEnrotePdu();
						if (replacePduData)
						{
							__tc1__("replaceSm.replaceRepeatedDeliveryEnrote");
							pdu->set_messageId(replacePduData->smsId.c_str());
						}
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			//��������� � ���������������� pdu
			if (pduReg)
			{
				if (sync)
				{
					PduData* pduData;
					{
						MutexGuard mguard(pduReg->getMutex());
						pdu->get_header().set_sequenceNumber(0); //�� ��������
						pduData = registerReplaceSm(pdu, replacePduData);
					}
					__dumpReplaceSmPdu__("replaceSmSyncBefore", systemId, pdu);
					PduReplaceSmResp* respPdu = session->getSyncTransmitter()->replace(*pdu);
					__dumpReplaceSmPdu__("replaceSmSyncAfter", systemId, pdu);
					{
						MutexGuard mguard(pduReg->getMutex());
						processReplaceSmSync(pduData, respPdu);
					}
				}
				else
				{
					MutexGuard mguard(pduReg->getMutex());
					__dumpReplaceSmPdu__("replaceSmAsyncBefore", systemId, pdu);
					PduReplaceSmResp* respPdu = session->getAsyncTransmitter()->replace(*pdu);
					__dumpReplaceSmPdu__("replaceSmAsyncAfter", systemId, pdu);
					PduData* pduData = registerReplaceSm(pdu, replacePduData);
					processReplaceSmAsync(pduData, respPdu);
				}
				//pdu life time ������������ PduRegistry
				//disposePdu(pdu);
			}
			else
			{
				if (sync)
				{
					PduReplaceSmResp* respPdu =
						session->getSyncTransmitter()->replace(*pdu);
					if (respPdu)
					{
						delete respPdu; //disposePdu
					}
				}
				else
				{
					session->getAsyncTransmitter()->replace(*pdu);
				}
				delete pdu; //disposePdu
			}
		}
		catch(...)
		{
			__tc12_fail__(s.value());
			error();
		}
	}
}

}
}
}

