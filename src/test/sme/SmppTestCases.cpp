#include "SmppTestCases.hpp"
#include "test/TestConfig.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::core::synchronization::MutexGuard;
using namespace smsc::test; //config constants
using namespace smsc::smpp::SmppCommandSet;

SmppTestCases::SmppTestCases(const SmeConfig& _config, const SmeSystemId& _systemId,
	const Address& _smeAddr, SmppResponseSender* respSender,
	const SmeRegistry* _smeReg, const AliasRegistry* _aliasReg,
	const RouteRegistry* _routeReg, ProfileRegistry* _profileReg,
	CheckList* _chkList)
	: config(_config), session(NULL), systemId(_systemId), smeAddr(_smeAddr),
	smeReg(_smeReg), aliasReg(_aliasReg), routeReg(_routeReg),
	profileReg(_profileReg), chkList(_chkList), routeChecker(NULL), pduChecker(NULL)
{
	__require__(respSender);
	__require__(smeReg);
	//__require__(aliasReg);
	//__require__(routeReg);
	//__require__(profileReg);
	//__require__(chkList);
	pduReg = smeReg->getPduRegistry(smeAddr); //может быть NULL
	if (pduReg && aliasReg && routeReg)
	{
		routeChecker = new RouteChecker(systemId, smeAddr, smeReg, aliasReg, routeReg);
		pduChecker = new SmppPduChecker(pduReg, routeChecker, chkList);
	}
	receiver = new SmppReceiverTestCases(systemId, smeAddr, respSender, smeReg,
		aliasReg, routeReg, profileReg, routeChecker, pduChecker, chkList);
	session = new SmppSession(config, receiver);
	receiver->setSession(session);
	transmitter = new SmppTransmitterTestCases(session, systemId, smeAddr,
		smeReg, profileReg, routeChecker, pduChecker, chkList);
}

SmppTestCases::~SmppTestCases()
{
	if (session)
	{
		if (routeChecker) { delete routeChecker; }
		if (pduChecker) { delete pduChecker; }
		delete receiver;
		delete transmitter;
		try
		{
			session->close();
		}
		catch(...)
		{
			//nothing
		}
		delete session;
	}
}

SmppReceiverTestCases& SmppTestCases::getReceiver()
{
	__require__(receiver);
	return *receiver;
}

SmppTransmitterTestCases& SmppTestCases::getTransmitter()
{
	__require__(transmitter);
	return *transmitter;
}

Category& SmppTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppTestCases");
	return log;
}

bool SmppTestCases::bindCorrectSme(int num)
{
	TCSelector s(num, 3);
	__decl_tc__;
	for (; s.check(); s++)
	{
		try
		{
			switch(s.value())
			{
				case 1: //BIND_RECIEVER
					__tc__("bindCorrectSme.bindReceiver");
					__tc_fail__(1);
					break;
				case 2: //BIND_TRANSMITTER
					__tc__("bindCorrectSme.bindTransmitter");
					__tc_fail__(2);
					break;
				case 3: //BIND_TRANCIEVER
					__tc__("bindCorrectSme.bindTransceiver");
					break;
				default:
					__unreachable__("Invalid num");
			}
			session->connect();
			__tc_ok_cond__;
			return true;
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
			return false;
		}
	}
}

void SmppTestCases::bindIncorrectSme(int num)
{
	//повторный bind убран из тест кейсов, т.к. приводит к пересозданию коннекта
	//без ошибок
	TCSelector s(num, 3);
	__decl_tc__;
	for (; s.check(); s++)
	{
		try
		{
			switch(s.value())
			{
				case 1: //sme не зарегистрирована в SC
					{
						__tc__("bindIncorrectSme.smeNotRegistered");
						SmppSession* sess = NULL;
						try
						{
							SmeConfig conf(config);
							auto_ptr<char> tmp = rand_char(15); //15 по спецификации
							conf.sid = tmp.get();
							SmppSession* sess = new SmppSession(conf, receiver);
							sess->connect();
							delete sess;
						}
						catch(...)
						{
							if (sess) { delete sess; }
							throw;
						}
					}
					break;
				case 2: //bind на недоступный SC (неизвестный хост)
					{
						__tc__("bindIncorrectSme.unknownHost");
						SmppSession* sess = NULL;
						try
						{
							SmeConfig conf(config);
							auto_ptr<char> tmp = rand_char(15);
							conf.host = tmp.get();
							SmppSession* sess = new SmppSession(conf, receiver);
							sess->connect();
							delete sess;
						}
						catch(...)
						{
							if (sess) { delete sess; }
							throw;
						}
					}
					break;
				case 3: //bind на недоступный SC (неправильный порт)
					{
						__tc__("bindIncorrectSme.invalidPort");
						SmppSession* sess = NULL;
						try
						{
							SmeConfig conf(config);
							auto_ptr<char> tmp = rand_char(15);
							conf.port += rand1(65535 - conf.port);
							SmppSession* sess = new SmppSession(conf, receiver);
							sess->connect();
							delete sess;
						}
						catch(...)
						{
							if (sess) { delete sess; }
							throw;
						}
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			__tc_fail__(s.value());
		}
		catch(...)
		{
			__tc_ok__;
		}
	}
}

#define __checkPdu__(tc) \
	__trace2__("%s(): checking pdu with msgRef = %u, submitTime = %ld, waitTime = %ld, validTime = %ld", \
	tc, (uint32_t) pduData->msgRef, pduData->submitTime, pduData->waitTime, pduData->validTime)

#define __missingPdu__(tc, pduName) \
	__trace2__("%s(): missing %s for msgRef = %u, submitTime = %ld, waitTime = %ld, validTime = %ld", \
	tc, pduName, (uint32_t) pduData->msgRef, pduData->submitTime, pduData->waitTime, pduData->validTime)
	/*
	static const char* fmt = "%Y-%m-%d %H:%M:%S"; \
	tm t;
	char __submitTime[20]; char __waitTime[20]; char __validTime[20]; \
	strftime(__submitTime, 20, fmt, localtime_r(&pduData->submitTime, &t)); \
	strftime(__waitTime, 20, fmt, localtime_r(&pduData->waitTime, &t)); \
	strftime(__validTime, 20, fmt, localtime_r(&pduData->validTime, &t)); \
	__trace2__("%s(): missing %s for sequenceNumber = %u, submitTime = %s, waitTime = %s, validTime = %s", \
	tc, pduName, pduData->pdu->get_sequenceNumber(), __submitTime, __waitTime, __validTime)
	*/

#define __removedPdu__(tc) \
	__trace2__("%s(): removed pdu data with msgRef = %u", \
	tc, (uint32_t) pduData->msgRef)
	
#define __checkSummary__(tc) \
	__trace2__("%s(): found = %d, deleted = %d", tc, found, deleted);

void SmppTestCases::checkSubmitTime(time_t checkTime)
{
	__decl_tc__;
	//проверить неполученные респонсы 
	PduRegistry::PduDataIterator* it = pduReg->getPduBySubmitTime(0, checkTime);
	int found = 0;
	int deleted = 0;
	while (PduData* pduData = it->next())
	{
		__checkPdu__("checkSubmitTime");
		found++;
		__tc__("checkMissingPdu.response");
		if (pduData->responseFlag == PDU_REQUIRED_FLAG)
		{
			__missingPdu__("checkSubmitTime", "response");
			__tc_fail__(1);
			pduData->responseFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		__tc_ok_cond__;
		/*
		if (pduData->complete())
		{
			 __removedPdu__("checkSubmitTime");
			deleted++;
			pduReg->removePdu(pduData);
		}
		*/
	}
	delete it;
	__checkSummary__("checkSubmitTime");
}

void SmppTestCases::checkWaitTime(time_t checkTime)
{
	__decl_tc__;
	//проверить неполученные доставки и подтверждения доставки 
	//на момент waitTime
	PduRegistry::PduDataIterator* it = pduReg->getPduByWaitTime(0, checkTime);
	int found = 0;
	int deleted = 0;
	while (PduData* pduData = it->next())
	{
		__checkPdu__("checkWaitTime");
		found++;
		__require__(pduData->pdu && pduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
		if (!routeChecker->isDestReachable(pdu->get_message().get_source(),
			pdu->get_message().get_dest(), true))
		{
			continue;
		}
		__tc__("checkMissingPdu.delivery.waitTime");
		if (pduData->deliveryFlag.isPduMissing(checkTime))
		{
			__missingPdu__("checkWaitTime", "deliver_sm");
			__tc_fail__(1);
			pduData->deliveryFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		__tc_ok_cond__;
		__tc__("checkMissingPdu.deliveryReceipt.waitTime");
		if (pduData->deliveryReceiptFlag.isPduMissing(checkTime))
		{
			__missingPdu__("checkWaitTime", "delivery_receipt");
			__tc_fail__(1);
			pduData->deliveryReceiptFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		__tc_ok_cond__;
		__tc__("checkMissingPdu.intermediateNotification.waitTime");
		if (pduData->intermediateNotificationFlag == PDU_REQUIRED_FLAG)
		{
			__missingPdu__("checkWaitTime", "intermediate_notification");
			__tc_fail__(1);
			pduData->intermediateNotificationFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		__tc_ok_cond__;
		/*
		if (pduData->complete())
		{
			__removedPdu__("checkWaitTime");
			deleted++;
			pduReg->removePdu(pduData);
		}
		*/
	}
	delete it;
	__checkSummary__("checkWaitTime");
}

void SmppTestCases::checkValidTime(time_t checkTime)
{
	__decl_tc__;
	//проверить неполученные доставки и подтверждения доставки
	//по окончании validTime
	PduRegistry::PduDataIterator* it = pduReg->getPduByValidTime(0, checkTime);
	int found = 0;
	int deleted = 0;
	while (PduData* pduData = it->next())
	{
		__checkPdu__("checkValidTime");
		found++;
		__require__(pduData->pdu && pduData->pdu->get_commandId() == SUBMIT_SM);
		PduSubmitSm* pdu = reinterpret_cast<PduSubmitSm*>(pduData->pdu);
		if (!routeChecker->isDestReachable(pdu->get_message().get_source(),
			pdu->get_message().get_dest(), true))
		{
			pduData->deliveryFlag = PDU_NOT_EXPECTED_FLAG;
		}
		__tc__("checkMissingPdu.delivery.validTime");
		if (pduData->deliveryFlag.isPduMissing(checkTime))
		{
			__missingPdu__("checkValidTime", "deliver_sm");
			__tc_fail__(1);
			pduData->deliveryFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		__tc_ok_cond__;
		__tc__("checkMissingPdu.deliveryReceipt.validTime");
		if (pduData->deliveryReceiptFlag.isPduMissing(checkTime))
		{
			__missingPdu__("checkValidTime", "delivery_receipt");
			__tc_fail__(1);
			pduData->deliveryReceiptFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		__tc_ok_cond__;
		__tc__("checkMissingPdu.intermediateNotification.validTime");
		if (pduData->intermediateNotificationFlag == PDU_REQUIRED_FLAG)
		{
			__missingPdu__("checkValidTime", "intermediate_notification");
			__tc_fail__(1);
			pduData->intermediateNotificationFlag = PDU_MISSING_ON_TIME_FLAG;
		}
		__tc_ok_cond__;
		//для pdu с validTime меньше текущего мог быть неполучен респонс
		if (pduData->submitTime <= checkTime)
		{
			__removedPdu__("checkValidTime");
			deleted++;
			pduReg->removePdu(pduData);
		}
	}
	delete it;
	__checkSummary__("checkValidTime");
}

void SmppTestCases::checkMissingPdu()
{
	if (pduReg)
	{
		MutexGuard mguard(pduReg->getMutex());
		__cfg_int__(timeCheckAccuracy);
		time_t checkTime = time(NULL) - timeCheckAccuracy;
		checkSubmitTime(checkTime);
		checkWaitTime(checkTime);
		checkValidTime(checkTime);
		__trace2__("checkMissingPdu(): pduReg size = %d", pduReg->size());
	}
}

void SmppTestCases::unbind()
{
	__decl_tc__;
	__tc__("unbind");
	try
	{
		session->close();
		__tc_ok__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

}
}
}

