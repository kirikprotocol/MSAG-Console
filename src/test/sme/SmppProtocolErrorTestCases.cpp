#include "SmppProtocolErrorTestCases.hpp"
#include "test/smpp/SmppUtil.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::test::smpp;
using namespace smsc::test::util;

void SmppProtocolErrorScenario::checkBindResp(PduBindTRXResp* pdu)
{
	__decl_tc__;
	__tc__("bindCorrectSme.checkCommandStatus");
	if (pdu->get_header().get_commandStatus() != ESME_ROK)
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
	__tc__("bindCorrectSme.checkInterfaceVersion");
	if (pdu->get_scInterfaceVersion() != 0x34)
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
}

void SmppProtocolErrorScenario::checkUnbindResp(PduUnbindResp* pdu)
{
	__decl_tc__;
	__tc__("unbind.checkCommandStatus");
	if (pdu->get_header().get_commandStatus() != ESME_ROK)
	{
		__tc_fail__(1);
	}
	__tc_ok_cond__;
}

inline void SmppProtocolErrorScenario::connect()
{
	__trace2__("connect(): scenario = %p", this);
	sess.connect(cfg.host.c_str(), cfg.port, cfg.timeOut);
}

inline void SmppProtocolErrorScenario::close()
{
	__trace2__("close(): scenario = %p", this);
	sess.close();
}

inline void SmppProtocolErrorScenario::sendPdu(SmppHeader* pdu)
{
	__trace2__("sendPdu(): scenario = %p", this);
	sess.sendPdu(pdu);
}

inline bool SmppProtocolErrorScenario::checkComplete(int timeout)
{
	__trace2__("waitComplete(): timeout = %d", timeout);
	if (timeout)
	{
		event.Wait(timeout);
	}
	return complete;
}

inline void SmppProtocolErrorScenario::setComplete(bool val)
{
	complete = val;
	if (complete)
	{
		event.Signal();
	}
}

SmppHeader* SmppProtocolErrorScenario::createPdu(uint32_t commandId)
{
	SmppHeader* pdu;
	switch (commandId)
	{
		case GENERIC_NACK:
			pdu = reinterpret_cast<SmppHeader*>(new PduGenericNack());
			break;
		case BIND_RECIEVER:
		case BIND_TRANSMITTER:
		case BIND_TRANCIEVER:
			pdu = reinterpret_cast<SmppHeader*>(new PduBindTRX());
			break;
		case BIND_RECIEVER_RESP:
		case BIND_TRANSMITTER_RESP:
		case BIND_TRANCIEVER_RESP:
			pdu = reinterpret_cast<SmppHeader*>(new PduBindTRXResp());
			break;
		case QUERY_SM:
			pdu = reinterpret_cast<SmppHeader*>(new PduQuerySm());
			break;
		case QUERY_SM_RESP:
			pdu = reinterpret_cast<SmppHeader*>(new PduQuerySmResp());
			break;
		case SUBMIT_SM:
			pdu = reinterpret_cast<SmppHeader*>(new PduSubmitSm());
			break;
		case SUBMIT_SM_RESP:
			pdu = reinterpret_cast<SmppHeader*>(new PduSubmitSmResp());
			break;
		case DELIVERY_SM:
			pdu = reinterpret_cast<SmppHeader*>(new PduDeliverySm());
			break;
		case DELIVERY_SM_RESP:
			pdu = reinterpret_cast<SmppHeader*>(new PduDeliverySmResp());
			break;
		case UNBIND:
			pdu = reinterpret_cast<SmppHeader*>(new PduUnbind());
			break;
		case UNBIND_RESP:
			pdu = reinterpret_cast<SmppHeader*>(new PduUnbindResp());
			break;
		case REPLACE_SM:
			pdu = reinterpret_cast<SmppHeader*>(new PduReplaceSm());
			break;
		case REPLACE_SM_RESP:
			pdu = reinterpret_cast<SmppHeader*>(new PduReplaceSmResp());
			break;
		case CANCEL_SM:
			pdu = reinterpret_cast<SmppHeader*>(new PduCancelSm());
			break;
		case CANCEL_SM_RESP:
			pdu = reinterpret_cast<SmppHeader*>(new PduCancelSmResp());
			break;
		case OUTBIND:
			pdu = reinterpret_cast<SmppHeader*>(new PduOutBind());
			break;
		case ENQUIRE_LINK:
			pdu = reinterpret_cast<SmppHeader*>(new PduEnquireLink());
			break;
		case ENQUIRE_LINK_RESP:
			pdu = reinterpret_cast<SmppHeader*>(new PduEnquireLinkResp());
			break;
		case SUBMIT_MULTI:
			pdu = reinterpret_cast<SmppHeader*>(new PduMultiSm());
			break;
		case SUBMIT_MULTI_RESP:
			pdu = reinterpret_cast<SmppHeader*>(new PduMultiSmResp());
			break;
		case ALERT_NOTIFICATION:
			pdu = reinterpret_cast<SmppHeader*>(new PduAlertNotification());
			break;
		case DATA_SM:
			pdu = reinterpret_cast<SmppHeader*>(new PduDataSm());
			break;
		case DATA_SM_RESP:
			pdu = reinterpret_cast<SmppHeader*>(new PduDataSmResp());
			break;
		default:
			__unreachable__("invalid commandId");
	}
	pdu->set_commandId(commandId);
	return pdu;
}

inline SmppHeader* SmppProtocolErrorScenario::setupBindPdu(PduBindTRX& pdu)
{
	__trace2__("setupBindPdu(): scenario = %p", this);
	pdu.get_header().set_commandId(SmppCommandSet::BIND_TRANCIEVER);
	pdu.set_systemId(cfg.sid.c_str());
	pdu.set_password(cfg.password.c_str());
	pdu.set_systemType(cfg.systemType.c_str());
	pdu.get_header().set_sequenceNumber(sess.getNextSeq());
	return reinterpret_cast<SmppHeader*>(&pdu);
}

inline SmppHeader* SmppProtocolErrorScenario::setupUnbindPdu(PduUnbind& pdu)
{
	__trace2__("setupUnbindPdu(): scenario = %p", this);
	pdu.get_header().set_commandId(SmppCommandSet::UNBIND);
	pdu.get_header().set_sequenceNumber(sess.getNextSeq());
	return reinterpret_cast<SmppHeader*>(&pdu);
}

inline SmppHeader* SmppProtocolErrorScenario::setupSubmitSmPdu(PduSubmitSm& pdu)
{
	__trace2__("setupSubmitSmPdu(): scenario = %p", this);
	SmppUtil::setupRandomCorrectSubmitSmPdu(&pdu, OPT_ALL);
	pdu.get_header().set_commandId(SmppCommandSet::SUBMIT_SM);
	pdu.get_header().set_sequenceNumber(sess.getNextSeq());
	PduAddress addr;
	SmppUtil::convert(smeAddr, &addr);
	pdu.get_message().set_source(addr);
	pdu.get_message().set_dest(addr);
	pdu.get_message().set_esmClass(ESM_CLASS_NORMAL_MESSAGE);
	pdu.get_message().set_scheduleDeliveryTime("");
	pdu.get_message().set_registredDelivery(0);
	return reinterpret_cast<SmppHeader*>(&pdu);
}

inline SmppHeader* SmppProtocolErrorScenario::setupDeliverySmRespPdu(
	PduDeliverySmResp& pdu, uint32_t seqNum)
{
	__trace2__("setupDeliverySmRespPdu(): scenario = %p", this);
	pdu.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
	pdu.get_header().set_sequenceNumber(seqNum);
	pdu.get_header().set_commandStatus(ESME_ROK);
	return reinterpret_cast<SmppHeader*>(&pdu);
}

inline SmppHeader* SmppProtocolErrorScenario::setupGenericNackPdu(PduGenericNack& pdu,
	uint32_t seqNum)
{
	pdu.get_header().set_commandId(GENERIC_NACK);
	pdu.get_header().set_sequenceNumber(seqNum);
	pdu.get_header().set_commandStatus(rand1(INT_MAX));
	__trace2__("setupGenericNackPdu(): scenario = %p", this);
	return reinterpret_cast<SmppHeader*>(&pdu);
}

SmppProtocolErrorTestCases::SmppProtocolErrorTestCases(const SmeConfig& _cfg,
	const Address& _smeAddr, CheckList* _chkList)
: cfg(_cfg), smeAddr(_smeAddr), chkList(_chkList) {}

Category& SmppProtocolErrorTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppProtocolErrorTestCases");
	return log;
}

#define __check__(errCode, cond) \
	if (!(cond)) { __tc_fail__(errCode); }
	
//invalid bind
class InvalidBindScenario : public SmppProtocolErrorScenario
{
	int num;
	bool invalidSize;
	bool invalidCmdId;
public:
	InvalidBindScenario(const SmeConfig& conf, const Address& addr,
		CheckList* chkList, int _num)
	: SmppProtocolErrorScenario(conf, addr, chkList), num(_num), invalidSize(false),
		invalidCmdId(false)
	{
		__tc__("protocolError.invalidBind");
	}
	virtual void execute()
	{
		static const uint32_t invalidCmdIds[] = {
			GENERIC_NACK,
			//BIND_RECIEVER,
			BIND_RECIEVER_RESP,
			//BIND_TRANSMITTER,
			BIND_TRANSMITTER_RESP,
			QUERY_SM,
			QUERY_SM_RESP,
			SUBMIT_SM,
			SUBMIT_SM_RESP,
			DELIVERY_SM,
			DELIVERY_SM_RESP,
			UNBIND,
			UNBIND_RESP,
			REPLACE_SM,
			REPLACE_SM_RESP,
			CANCEL_SM,
			CANCEL_SM_RESP,
			//BIND_TRANCIEVER,
			BIND_TRANCIEVER_RESP,
			OUTBIND,
			ENQUIRE_LINK,
			ENQUIRE_LINK_RESP,
			SUBMIT_MULTI,
			SUBMIT_MULTI_RESP,
			ALERT_NOTIFICATION,
			DATA_SM,
			DATA_SM_RESP
		};
		static const uint32_t correctCmdIds[] = {
			BIND_RECIEVER,
			BIND_TRANSMITTER,
			BIND_TRANCIEVER
		};
		static const int invalidCmdIdsSize = sizeof(invalidCmdIds) / sizeof(*invalidCmdIds);
		static const int correctCmdIdsSize = sizeof(correctCmdIds) / sizeof(*correctCmdIds);
		//connect
		connect();
		//������������ bind
		TCSelector s(num, 5);
		for (; s.check(); s++)
		{
			SmppHeader* pdu = NULL;
			int size;
			uint32_t cmdId;
			switch (s.value())
			{
				case 1: //������ ������� ������
					__tc__("protocolError.invalidBind.smallerSize1");
					cmdId = correctCmdIds[rand0(correctCmdIdsSize - 1)];
					pdu = createPdu(cmdId);
					size = rand0(15);
					invalidSize = true;
					break;
				case 2: //������ ������������� �������
					__tc__("protocolError.invalidBind.smallerSize2");
					cmdId = correctCmdIds[rand0(correctCmdIdsSize - 1)];
					pdu = createPdu(cmdId);
					size = rand2(16, calcSmppPacketLength(pdu) - 1);
					invalidSize = true;
					break;
				case 3: //������ ������������� �������
					__tc__("protocolError.invalidBind.greaterSize");
					cmdId = correctCmdIds[rand0(correctCmdIdsSize - 1)];
					pdu = createPdu(cmdId);
					size = rand2(calcSmppPacketLength(pdu) + 1, INT_MAX);
					invalidSize = true;
					break;
				case 4: //������������ commandId
					__tc__("protocolError.submitWithoutBind.invalidCommandId");
					cmdId = invalidCmdIds[rand0(invalidCmdIdsSize - 1)];
					pdu = createPdu(cmdId);
					invalidCmdId = true;
					break;
				case 5:  //�������������� commandId
					__tc__("protocolError.submitWithoutBind.nonExistentCommandId");
					cmdId = rand2(0xa, INT_MAX);
					pdu = createPdu(correctCmdIds[rand0(correctCmdIdsSize - 1)]);
					invalidCmdId = true;
					break;
				default:
					__unreachable__("Invalid num");
			}
			__require__(pdu);
			PduBuffer pb = sess.getBytes(pdu);
			disposePdu(pdu);
			uint32_t* tmp = (uint32_t*) pb.buf;
			if (invalidSize)
			{
				__trace2__("invalid bind: scenario = %p, command id = %x, original size = %d, new size = %d",
					this, cmdId, pb.size, size);
				*tmp = htonl(size);
			}
			if (invalidCmdId)
			{
				__trace2__("invalid bind: scenario = %p, new command id = %x",
					this, cmdId);
				*(tmp + 1) = htonl(cmdId);
			}
			sess.sendBytes(pb);
		}
		if (!checkComplete(10000))
		{
			__tc_fail__(1);
		}
		__tc_ok_cond__;
	}
	virtual void handleEvent(SmppHeader *pdu)
	{
		switch (pdu->get_commandId())
		{
			case GENERIC_NACK:
				__check__(2, invalidCmdId);
				__check__(3, pdu->get_commandStatus() == ESME_RINVBNDSTS);
				setComplete(true);
				break;
			default:
				__tc_fail__(4);
		}
	}
	virtual void handleError(int errorCode)
	{
		__check__(5, invalidSize);
		setComplete(true);
	}
};

void SmppProtocolErrorTestCases::invalidBindScenario(int num)
{
	InvalidBindScenario scenario(cfg, smeAddr, chkList, num);
	scenario.execute();
}

//corrupted pdu
class InvalidPduScenario : public SmppProtocolErrorScenario
{
	int num;
	bool bound;
	bool invalidSize;
	bool invalidCmdId;
public:
	InvalidPduScenario(const SmeConfig& conf, const Address& addr,
		CheckList* chkList, int _num)
	: SmppProtocolErrorScenario(conf, addr, chkList), num(_num), bound(false),
		invalidSize(false), invalidCmdId(false)
	{
		__tc__("protocolError.invalidPdu");
	}
	virtual void execute()
	{
		static const uint32_t invalidCmdIds[] = {
			//GENERIC_NACK,
			//BIND_RECIEVER,
			BIND_RECIEVER_RESP,
			//BIND_TRANSMITTER,
			BIND_TRANSMITTER_RESP,
			//QUERY_SM,
			QUERY_SM_RESP,
			//SUBMIT_SM,
			SUBMIT_SM_RESP,
			DELIVERY_SM,
			//DELIVERY_SM_RESP,
			//UNBIND,
			//UNBIND_RESP, <- ���� ����� sme ����������
			//REPLACE_SM,
			REPLACE_SM_RESP,
			//CANCEL_SM,
			CANCEL_SM_RESP,
			//BIND_TRANCIEVER,
			BIND_TRANCIEVER_RESP,
			OUTBIND,
			//ENQUIRE_LINK,
			//ENQUIRE_LINK_RESP,
			//SUBMIT_MULTI,
			SUBMIT_MULTI_RESP,
			ALERT_NOTIFICATION,
			//DATA_SM,
			//DATA_SM_RESP
		};
		static const uint32_t correctCmdIds[] = {
			GENERIC_NACK,
			BIND_RECIEVER,
			//BIND_RECIEVER_RESP,
			BIND_TRANSMITTER,
			//BIND_TRANSMITTER_RESP,
			QUERY_SM,
			//QUERY_SM_RESP,
			SUBMIT_SM,
			//SUBMIT_SM_RESP,
			//DELIVERY_SM,
			DELIVERY_SM_RESP,
			UNBIND,
			//UNBIND_RESP,
			REPLACE_SM,
			//REPLACE_SM_RESP,
			CANCEL_SM,
			//CANCEL_SM_RESP,
			BIND_TRANCIEVER,
			//BIND_TRANCIEVER_RESP,
			//OUTBIND,
			ENQUIRE_LINK,
			ENQUIRE_LINK_RESP,
			SUBMIT_MULTI,
			//SUBMIT_MULTI_RESP,
			//ALERT_NOTIFICATION,
			DATA_SM,
			DATA_SM_RESP
		};
		static const int correctCmdIdsSize = sizeof(correctCmdIds) / sizeof(*correctCmdIds);
		static const int invalidCmdIdsSize = sizeof(invalidCmdIds) / sizeof(*invalidCmdIds);
		//connect & bind
		connect();
		PduBindTRX bindPdu;
		sendPdu(setupBindPdu(bindPdu));
		//������������ pdu
		TCSelector s(num, 5);
		for (; s.check(); s++)
		{
			SmppHeader* pdu = NULL;
			int size;
			uint32_t cmdId;
			switch (s.value())
			{
				case 1: //������ ������� ������
					__tc__("protocolError.invalidPdu.smallerSize1");
					cmdId = correctCmdIds[rand0(correctCmdIdsSize - 1)];
					pdu = createPdu(cmdId);
					size = rand0(15);
					invalidSize = true;
					break;
				case 2: //������ ������������� �������
					__tc__("protocolError.invalidPdu.smallerSize2");
					cmdId = correctCmdIds[rand0(correctCmdIdsSize - 1)];
					pdu = createPdu(cmdId);
					size = calcSmppPacketLength(pdu) == 16 ? rand0(15) :
						rand2(16, calcSmppPacketLength(pdu) - 1);
					invalidSize = true;
					break;
				case 3: //������ ������������� �������
					__tc__("protocolError.invalidPdu.greaterSize");
					cmdId = correctCmdIds[rand0(correctCmdIdsSize - 1)];
					pdu = createPdu(cmdId);
					size = rand2(calcSmppPacketLength(pdu) + 1, INT_MAX);
					invalidSize = true;
					break;
				case 4: //������������ commandId
					__tc__("protocolError.invalidPdu.invalidCommandId");
					cmdId = invalidCmdIds[rand0(invalidCmdIdsSize - 1)];
					pdu = createPdu(cmdId);
					invalidCmdId = true;
					break;
				case 5:  //�������������� commandId
					__tc__("protocolError.invalidPdu.nonExistentCommandId");
					cmdId = rand2(0xa, INT_MAX);
					pdu = createPdu(correctCmdIds[rand0(correctCmdIdsSize - 1)]);
					invalidCmdId = true;
					break;
				default:
					__unreachable__("Invalid num");
			}
			__require__(pdu);
			PduBuffer pb = sess.getBytes(pdu);
			disposePdu(pdu);
			uint32_t* tmp = (uint32_t*) pb.buf;
			if (invalidSize)
			{
				__trace2__("invalid pdu: scenario = %p, command id = %x, original size = %d, new size = %d",
					this, cmdId, pb.size, size);
				*tmp = htonl(size);
			}
			if (invalidCmdId)
			{
				__trace2__("invalid pdu: scenario = %p, new command id = %x",
					this, cmdId);
				*(tmp + 1) = htonl(cmdId);
			}
			sess.sendBytes(pb);
		}
		if (invalidSize)
		{
			//��� ��������� ���������� pdu
			for (int i = 0; i < 10; i++)
			{
				try
				{
					PduSubmitSm pdu;
					sendPdu(setupSubmitSmPdu(pdu));
					sleep(1);
				}
				catch (...)
				{
					break;
				}
			}
		}
		if (!checkComplete(10000))
		{
			__tc_fail__(1);
		}
		__tc_ok_cond__;
	}
	virtual void handleEvent(SmppHeader* pdu)
	{
		switch (pdu->get_commandId())
		{
			case BIND_RECIEVER_RESP:
			case BIND_TRANSMITTER_RESP:
			case BIND_TRANCIEVER_RESP:
				__check__(2, !bound);
				bound = true;
				checkBindResp(reinterpret_cast<PduBindTRXResp*>(pdu));
				break;
			case GENERIC_NACK:
				__check__(3, invalidCmdId);
				__check__(4, pdu->get_commandStatus() == ESME_RINVCMDID);
				setComplete(true);
				break;
			default:
				__tc_fail__(5);
		}
	}
	virtual void handleError(int errorCode)
	{
		__check__(6, invalidSize);
		setComplete(true);
	}
};

void SmppProtocolErrorTestCases::invalidPduScenario(int num)
{
	InvalidPduScenario scenario(cfg, smeAddr, chkList, num);
	scenario.execute();
}

class EqualSequenceNumbersScenario : public SmppProtocolErrorScenario
{
	static const int pduCount = 10;
	int respCount;
	int deliveryCount;
	bool bound;
public:
	EqualSequenceNumbersScenario(const SmeConfig& conf, const Address& addr,
		CheckList* chkList)
	: SmppProtocolErrorScenario(conf, addr, chkList), respCount(0), deliveryCount(0),
		bound(false)
	{
		__tc__("protocolError.equalSeqNum");
	}
	virtual void execute()
	{
		//connect & bind
		connect();
		PduBindTRX bindPdu;
		sendPdu(setupBindPdu(bindPdu));
		//��������� pdu � seqNum = 0
		for (int i = 0; i < pduCount; i++)
		{
			PduSubmitSm pdu;
			SmppHeader* header = setupSubmitSmPdu(pdu);
			header->set_sequenceNumber(0);
			sendPdu(header);
		}
		if (!checkComplete(10000))
		{
			__tc_fail__(1);
		}
		setComplete(false);
		//unbind
		PduUnbind unbindPdu;
		sendPdu(setupUnbindPdu(unbindPdu));
		if (!checkComplete(10000))
		{
			__tc_fail__(1);
		}
		__tc_ok_cond__;
	}
	virtual void handleEvent(SmppHeader* pdu)
	{
		switch (pdu->get_commandId())
		{
			case BIND_RECIEVER_RESP:
			case BIND_TRANSMITTER_RESP:
			case BIND_TRANCIEVER_RESP:
				__check__(2, !bound);
				bound = true;
				checkBindResp(reinterpret_cast<PduBindTRXResp*>(pdu));
				break;
			case UNBIND_RESP:
				__check__(3, bound);
				bound = false;
				checkUnbindResp(reinterpret_cast<PduUnbindResp*>(pdu));
				setComplete(true);
				break;
			case DELIVERY_SM:
				{
					__trace2__("deliver_sm_resp: serviceType = %s, sequenceNumber = %u, scenario = %p",
						reinterpret_cast<PduDeliverySm*>(pdu)->get_message().get_serviceType(),
						pdu->get_sequenceNumber(), this);
					__check__(4, ++deliveryCount <= pduCount);
					PduDeliverySmResp respPdu;
					sendPdu(setupDeliverySmRespPdu(respPdu, pdu->get_sequenceNumber()));
				}
				break;
			case SUBMIT_SM_RESP:
				__check__(5, ++respCount <= pduCount);
				__check__(6, pdu->get_commandStatus() == ESME_ROK);
				__check__(7, pdu->get_sequenceNumber() == 0);
				break;
			default:
				__tc_fail__(8);
		}
		if (bound && respCount == pduCount && deliveryCount == pduCount)
		{
			setComplete(true);
		}
	}
	virtual void handleError(int errorCode)
	{
		//SC �� ��������� ����� ��� unbind
		__tc_fail__(9);
	}
};

void SmppProtocolErrorTestCases::equalSequenceNumbersScenario()
{
	EqualSequenceNumbersScenario scenario(cfg, smeAddr, chkList);
	scenario.execute();
}

class SubmitAfterUnbindScenario : public SmppProtocolErrorScenario
{
	bool bound;
public:
	SubmitAfterUnbindScenario(const SmeConfig& conf, const Address& addr,
		CheckList* chkList)
	: SmppProtocolErrorScenario(conf, addr, chkList), bound(false)
	{
		__tc__("protocolError.submitAfterUnbind");
	}
	virtual void execute()
	{
		//connect & bind
		connect();
		PduBindTRX bindPdu;
		sendPdu(setupBindPdu(bindPdu));
		//unbind
		PduUnbind unbindPdu;
		sendPdu(setupUnbindPdu(unbindPdu));
		//��� submit_sm
		PduSubmitSm pdu;
		sendPdu(setupSubmitSmPdu(pdu));
		if (!checkComplete(10000))
		{
			__tc_fail__(1);
		}
		__tc_ok_cond__;
	}
	virtual void handleEvent(SmppHeader* pdu)
	{
		__trace2__("handleEvent(): commandId = %x", pdu->get_commandId());
		switch (pdu->get_commandId())
		{
			case BIND_RECIEVER_RESP:
			case BIND_TRANSMITTER_RESP:
			case BIND_TRANCIEVER_RESP:
				__check__(2, !bound);
				bound = true;
				checkBindResp(reinterpret_cast<PduBindTRXResp*>(pdu));
				break;
			case UNBIND_RESP:
				__check__(3, bound);
				bound = false;
				checkUnbindResp(reinterpret_cast<PduUnbindResp*>(pdu));
				setComplete(true);
				break;
			default:
				__tc_fail__(4);
		}
	}
	virtual void handleError(int errorCode)
	{
		//SC �� ��������� ����� ��� unbind
		__tc_fail__(5);
	}
};

void SmppProtocolErrorTestCases::submitAfterUnbindScenario()
{
	SubmitAfterUnbindScenario scenario(cfg, smeAddr, chkList);
	scenario.execute();
}

}
}
}

