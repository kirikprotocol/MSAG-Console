#include "SmppProtocolErrorTestCases.hpp"
#include "test/smpp/SmppUtil.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using namespace smsc::sme;
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::test::smpp;
using namespace smsc::test::util;

static const uint32_t allowedCmdIds[] = {
	GENERIC_NACK,
	//BIND_RECIEVER,
	//BIND_RECIEVER_RESP,
	//BIND_TRANSMITTER,
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
	//BIND_TRANCIEVER,
	//BIND_TRANCIEVER_RESP,
	//OUTBIND,
	ENQUIRE_LINK,
	ENQUIRE_LINK_RESP,
//SUBMIT_MULTI,
	//SUBMIT_MULTI_RESP,
	//ALERT_NOTIFICATION,
	DATA_SM,
	DATA_SM_RESP
};

static const uint32_t notAllowedCmdIds[] = {
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
	UNBIND_RESP,
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
//SUBMIT_MULTI_RESP,
	ALERT_NOTIFICATION,
	//DATA_SM,
	//DATA_SM_RESP
};

static const uint32_t bindCmdIds[] = {
	BIND_RECIEVER,
	BIND_TRANSMITTER,
	BIND_TRANCIEVER
};

static const int allowedCmdIdsSize = sizeof(allowedCmdIds) / sizeof(*allowedCmdIds);
static const int notAllowedCmdIdsSize = sizeof(notAllowedCmdIds) / sizeof(*notAllowedCmdIds);
static const int bindCmdIdsSize = sizeof(bindCmdIds) / sizeof(*bindCmdIds);

#define __check__(errCode, cond) \
	if (!(cond)) { __tc_fail__(errCode); }
	
void SmppProtocolErrorScenario::checkBindResp(SmppHeader* pdu)
{
	__decl_tc__;
	switch (pdu->get_commandId())
	{
		case BIND_RECIEVER_RESP:
			__tc__("bind.resp.receiver");
			__check__(1, bindType == BindType::Receiver);
			break;
		case BIND_TRANSMITTER_RESP:
			__tc__("bind.resp.transmitter");
			__check__(1, bindType == BindType::Transmitter);
			break;
		case BIND_TRANCIEVER_RESP:
			__tc__("bind.resp.transceiver");
			__check__(1, bindType == BindType::Transceiver);
			break;
		default:
			__unreachable__("Invalid bind response");
	}
	__check__(2, !bound);
	bound = true;
	__tc_ok_cond__;
	__tc__("bind.resp.checkCommandStatus");
	__check__(1, pdu->get_commandStatus() == ESME_ROK);
	__tc_ok_cond__;
	__tc__("bind.resp.checkInterfaceVersion");
	__check__(1, reinterpret_cast<PduBindTRXResp*>(pdu)->get_scInterfaceVersion() == 0x34);
	__tc_ok_cond__;
}

void SmppProtocolErrorScenario::checkUnbindResp(SmppHeader* pdu)
{
	__decl_tc__;
	__tc__("unbind.resp.checkCommandStatus");
	__check__(1, pdu->get_commandStatus() == ESME_ROK);
	__check__(2, bound);
	bound = false;
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
	__trace2__("setComplete(): val = %s", val ? "true" : "false");
	complete = val;
	if (complete)
	{
		event.Signal();
	}
}

SmppHeader* SmppProtocolErrorScenario::createPdu(uint32_t commandId)
{
	__trace2__("createPdu(): scenario = %p, commandId = %x", this, commandId);
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
			{
				PduMultiSmResp* p = new PduMultiSmResp();
				p->sme = NULL;
				pdu = reinterpret_cast<SmppHeader*>(p);
			}
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

inline SmppHeader* SmppProtocolErrorScenario::setupBindPdu(PduBindTRX& pdu,
	int bindType)
{
	__trace2__("setupBindPdu(): scenario = %p", this);
	uint32_t cmdId;
	switch (bindType)
	{
		case BindType::Receiver:
			cmdId = BIND_RECIEVER;
			break;
		case BindType::Transmitter:
			cmdId = BIND_TRANSMITTER;
			break;
		case BindType::Transceiver:
			cmdId = BIND_TRANCIEVER;
			break;
		default:
			__unreachable__("Invalid bind type");
	}
	pdu.get_header().set_commandId(cmdId);
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
	pdu.get_message().set_replaceIfPresentFlag(0);
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

//invalid bind
class InvalidBindScenario : public SmppProtocolErrorScenario
{
	int num;
	bool invalidSize;
	bool allowedCmdId;
	bool notAllowedCmdId;
public:
	InvalidBindScenario(const SmeConfig& conf, const Address& addr,
		CheckList* chkList, int _num)
	: SmppProtocolErrorScenario(conf, addr, chkList), num(_num), invalidSize(false),
		allowedCmdId(false), notAllowedCmdId(false)
	{
		__tc__("protocolError.invalidBind");
		__trace2__("InvalidBindScenario(): scenario = %p", this);
	}
	~InvalidBindScenario()
	{
		__trace2__("~InvalidBindScenario(): scenario = %p", this);
	}
	virtual void execute()
	{
		//connect
		connect();
		//неправильный bind
		TCSelector s(num, 7);
		SmppHeader* pdu = NULL;
		int size;
		uint32_t cmdId;
		switch (s.value())
		{
			case 1: //меньше размера хедера
				__tc__("protocolError.invalidBind.smallerSize1");
				cmdId = bindCmdIds[rand0(bindCmdIdsSize - 1)];
				pdu = createPdu(cmdId);
				size = rand0(15);
				invalidSize = true;
				break;
			case 2: //меньше оригинального размера
				__tc__("protocolError.invalidBind.smallerSize2");
				cmdId = bindCmdIds[rand0(bindCmdIdsSize - 1)];
				pdu = createPdu(cmdId);
				size = rand2(16, calcSmppPacketLength(pdu) - 1);
				invalidSize = true;
				break;
			case 3: //больше оригинального размера
				__tc__("protocolError.invalidBind.greaterSize1");
				cmdId = bindCmdIds[rand0(bindCmdIdsSize - 1)];
				pdu = createPdu(cmdId);
				size = rand2(calcSmppPacketLength(pdu) + 1, 65535);
				invalidSize = true;
				break;
			case 4: //больше оригинального размера
				__tc__("protocolError.invalidBind.greaterSize2");
				cmdId = bindCmdIds[rand0(bindCmdIdsSize - 1)];
				pdu = createPdu(cmdId);
				size = rand2(100000, INT_MAX);
				invalidSize = true;
				break;
			case 5: //неправильный commandId
				__tc__("protocolError.invalidBind.allowedCommandId");
				cmdId = allowedCmdIds[rand0(allowedCmdIdsSize - 1)];
				pdu = createPdu(cmdId);
				allowedCmdId = true;
				break;
			case 6: //неправильный commandId
				__tc__("protocolError.invalidBind.notAllowedCommandId");
				cmdId = notAllowedCmdIds[rand0(notAllowedCmdIdsSize - 1)];
				pdu = createPdu(cmdId);
				notAllowedCmdId = true;
				break;
			case 7:  //несуществующий commandId
				__tc__("protocolError.invalidBind.nonExistentCommandId");
				cmdId = rand2(0xa, INT_MAX);
				pdu = createPdu(bindCmdIds[rand0(bindCmdIdsSize - 1)]);
				notAllowedCmdId = true;
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
		if (allowedCmdId || notAllowedCmdId)
		{
			__trace2__("invalid bind: scenario = %p, new command id = %x",
				this, cmdId);
			*(tmp + 1) = htonl(cmdId);
		}
		sess.sendBytes(pb);
		if (!checkComplete(10000))
		{
			__tc_fail__(1);
		}
		__tc_ok_cond__;
	}
	virtual void handleEvent(SmppHeader *pdu)
	{
		__trace2__("handleEvent(): commandId = %x, commandStatus = %x", pdu->get_commandId(), pdu->get_commandStatus());
		switch (pdu->get_commandId())
		{
			case GENERIC_NACK:
				if (allowedCmdId)
				{
					__check__(2, pdu->get_commandStatus() == ESME_RINVBNDSTS);
				}
				else if (notAllowedCmdId)
				{
					__check__(3, pdu->get_commandStatus() == ESME_RINVCMDID);
				}
				else
				{
					__tc_fail__(4);
				}
				setComplete(true);
				break;
			default:
				__tc_fail__(5);
		}
	}
	virtual void handleError(int errorCode)
	{
		__trace2__("handleError(): errorCode = %d", errorCode);
		__check__(6, invalidSize);
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
	bool invalidSize;
	bool invalidCmdId;
public:
	InvalidPduScenario(const SmeConfig& conf, const Address& addr,
		CheckList* chkList, int _num)
	: SmppProtocolErrorScenario(conf, addr, chkList), num(_num),
		invalidSize(false), invalidCmdId(false)
	{
		__tc__("protocolError.invalidPdu");
		__trace2__("InvalidPduScenario(): scenario = %p", this);
	}
	~InvalidPduScenario()
	{
		__trace2__("~InvalidPduScenario(): scenario = %p", this);
	}
	virtual void execute()
	{
		//connect & bind
		connect();
		PduBindTRX bindPdu;
		sendPdu(setupBindPdu(bindPdu, BindType::Transceiver));
		//неправильные pdu
		TCSelector s(num, 7);
		SmppHeader* pdu = NULL;
		int size;
		uint32_t cmdId;
		switch (s.value())
		{
			case 1: //меньше размера хедера
				__tc__("protocolError.invalidPdu.smallerSize1");
				cmdId = allowedCmdIds[rand0(allowedCmdIdsSize - 1)];
				pdu = createPdu(cmdId);
				size = rand0(15);
				invalidSize = true;
				break;
			case 2: //меньше оригинального размера
				__tc__("protocolError.invalidPdu.smallerSize2");
				cmdId = allowedCmdIds[rand0(allowedCmdIdsSize - 1)];
				pdu = createPdu(cmdId);
				size = calcSmppPacketLength(pdu) == 16 ? rand0(15) :
					rand2(16, calcSmppPacketLength(pdu) - 1);
				invalidSize = true;
				break;
			case 3: //больше оригинального размера
				__tc__("protocolError.invalidPdu.greaterSize1");
				cmdId = allowedCmdIds[rand0(allowedCmdIdsSize - 1)];
				pdu = createPdu(cmdId);
				size = rand2(calcSmppPacketLength(pdu) + 1, 65535);
				invalidSize = true;
				break;
			case 4: //больше оригинального размера
				__tc__("protocolError.invalidPdu.greaterSize2");
				cmdId = allowedCmdIds[rand0(allowedCmdIdsSize - 1)];
				pdu = createPdu(cmdId);
				size = rand2(100000, INT_MAX);
				invalidSize = true;
				break;
			case 5: //неправильный commandId
				__tc__("protocolError.invalidPdu.notAllowedCommandId");
				cmdId = notAllowedCmdIds[rand0(notAllowedCmdIdsSize - 1)];
				pdu = createPdu(cmdId);
				invalidCmdId = true;
				break;
			case 6: //неправильный commandId
				__tc__("protocolError.invalidPdu.notAllowedCommandId");
				cmdId = bindCmdIds[rand0(bindCmdIdsSize - 1)];
				pdu = createPdu(cmdId);
				invalidCmdId = true;
				break;
			case 7:  //несуществующий commandId
				__tc__("protocolError.invalidPdu.nonExistentCommandId");
				cmdId = rand2(0xa, INT_MAX);
				pdu = createPdu(allowedCmdIds[rand0(allowedCmdIdsSize - 1)]);
				invalidCmdId = true;
				break;
			default:
				__unreachable__("Invalid num");
		}
		__require__(pdu);
		PduBuffer pb = sess.getBytes(pdu);
		__require__(pb.size >= 16);
		__trace2__("before disposePdu: commandId = %x", pdu->get_commandId());
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
		/*
		if (invalidSize)
		{
			//еще несколько правильных pdu
			int numPdu = rand0(3);
			for (int i = 0; i < numPdu; i++)
			{
			__trace__("cycle");
				try
				{
					PduSubmitSm pdu;
					sendPdu(setupSubmitSmPdu(pdu));
					//sleep(1);
				}
				catch (...)
				{
					break;
				}
			}
		}
		*/
		if (!checkComplete(10000))
		{
			__tc_fail__(1);
		}
		__tc_ok_cond__;
	}
	virtual void handleEvent(SmppHeader* pdu)
	{
		__trace2__("handleEvent(): commandId = %x, commandStatus = %x", pdu->get_commandId(), pdu->get_commandStatus());
		switch (pdu->get_commandId())
		{
			case BIND_RECIEVER_RESP:
			case BIND_TRANSMITTER_RESP:
			case BIND_TRANCIEVER_RESP:
				checkBindResp(pdu);
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
		__trace2__("handleError(): errorCode = %d", errorCode);
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
public:
	EqualSequenceNumbersScenario(const SmeConfig& conf, const Address& addr,
		CheckList* chkList)
	: SmppProtocolErrorScenario(conf, addr, chkList), respCount(0),
		deliveryCount(0)
	{
		__tc__("protocolError.equalSeqNum");
		__trace2__("EqualSequenceNumbersScenario(): scenario = %p", this);
	}
	~EqualSequenceNumbersScenario()
	{
		__trace2__("~EqualSequenceNumbersScenario(): scenario = %p", this);
	}
	virtual void execute()
	{
		//connect & bind
		connect();
		PduBindTRX bindPdu;
		sendPdu(setupBindPdu(bindPdu, BindType::Transceiver));
		//несколько pdu с seqNum = 0
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
			return;
		}
		setComplete(false);
		//unbind
		sleep(1); //иначе на unbind реквест закроется прокси, а очередь обработаться не успеет
		PduUnbind unbindPdu;
		sendPdu(setupUnbindPdu(unbindPdu));
		if (!checkComplete(10000))
		{
			__tc_fail__(2);
		}
		__tc_ok_cond__;
	}
	virtual void handleEvent(SmppHeader* pdu)
	{
		__trace2__("handleEvent(): commandId = %x, commandStatus = %x", pdu->get_commandId(), pdu->get_commandStatus());
		switch (pdu->get_commandId())
		{
			case BIND_RECIEVER_RESP:
			case BIND_TRANSMITTER_RESP:
			case BIND_TRANCIEVER_RESP:
				checkBindResp(pdu);
				break;
			case UNBIND_RESP:
				checkUnbindResp(pdu);
				setComplete(true);
				break;
			case DELIVERY_SM:
				{
					__trace2__("deliver_sm_resp: serviceType = %s, sequenceNumber = %u, scenario = %p",
						reinterpret_cast<PduDeliverySm*>(pdu)->get_message().get_serviceType(),
						pdu->get_sequenceNumber(), this);
					__check__(5, ++deliveryCount <= pduCount);
					PduDeliverySmResp respPdu;
					sendPdu(setupDeliverySmRespPdu(respPdu, pdu->get_sequenceNumber()));
				}
				break;
			case SUBMIT_SM_RESP:
				__check__(6, ++respCount <= pduCount);
				__check__(7, pdu->get_commandStatus() == ESME_ROK);
				__check__(8, pdu->get_sequenceNumber() == 0);
				break;
			default:
				__tc_fail__(9);
		}
		if (bound && respCount == pduCount && deliveryCount == pduCount)
		{
			setComplete(true);
		}
	}
	virtual void handleError(int errorCode)
	{
		__trace2__("handleError(): errorCode = %d", errorCode);
		//SC не закрывает сокет при unbind
		__tc_fail__(10);
	}
};

void SmppProtocolErrorTestCases::equalSequenceNumbersScenario()
{
	EqualSequenceNumbersScenario scenario(cfg, smeAddr, chkList);
	scenario.execute();
}

class SubmitAfterUnbindScenario : public SmppProtocolErrorScenario
{
	int num;
	bool allowedCmdId;
	bool notAllowedCmdId;
public:
	SubmitAfterUnbindScenario(const SmeConfig& conf, const Address& addr,
		CheckList* chkList, int _num)
	: SmppProtocolErrorScenario(conf, addr, chkList), num(_num),
		allowedCmdId(false), notAllowedCmdId(false)

	{
		__tc__("protocolError.submitAfterUnbind");
		__trace2__("SubmitAfterUnbindScenario(): scenario = %p", this);
	}
	~SubmitAfterUnbindScenario()
	{
		__trace2__("~SubmitAfterUnbindScenario(): scenario = %p", this);
	}
	virtual void execute()
	{
		//connect & bind
		connect();
		PduBindTRX bindPdu;
		sendPdu(setupBindPdu(bindPdu, BindType::Transceiver));
		//unbind
		PduUnbind unbindPdu;
		sendPdu(setupUnbindPdu(unbindPdu));
		if (!checkComplete(10000))
		{
			__tc_fail__(1);
			return;
		}
		setComplete(false);
		//произвольные pdu
		TCSelector s(num, 3);
		SmppHeader* pdu = NULL;
		uint32_t cmdId;
		switch (s.value())
		{
			case 1: //неправильный commandId
				__tc__("protocolError.submitAfterUnbind.allowedCommandId");
				cmdId = allowedCmdIds[rand0(allowedCmdIdsSize - 1)];
				pdu = createPdu(cmdId);
				allowedCmdId = true;
				break;
			case 2: //неправильный commandId
				__tc__("protocolError.submitAfterUnbind.notAllowedCommandId");
				cmdId = notAllowedCmdIds[rand0(notAllowedCmdIdsSize - 1)];
				pdu = createPdu(cmdId);
				notAllowedCmdId = true;
				break;
			case 3:  //несуществующий commandId
				__tc__("protocolError.submitAfterUnbind.nonExistentCommandId");
				cmdId = rand2(0xa, INT_MAX);
				pdu = createPdu(bindCmdIds[rand0(bindCmdIdsSize - 1)]);
				notAllowedCmdId = true;
				break;
			default:
				__unreachable__("Invalid num");
		}
		__require__(pdu);
		PduBuffer pb = sess.getBytes(pdu);
		__require__(pb.size >= 16);
		disposePdu(pdu);
		__trace2__("invalid pdu: scenario = %p, new command id = %x", this, cmdId);
		uint32_t* tmp = (uint32_t*) pb.buf;
		*(tmp + 1) = htonl(cmdId);
		sess.sendBytes(pb);
		//PduSubmitSm pdu;
		//sendPdu(setupSubmitSmPdu(pdu));
		if (!checkComplete(10000))
		{
			__tc_fail__(2);
		}
		__tc_ok_cond__;
	}
	virtual void handleEvent(SmppHeader* pdu)
	{
		__trace2__("handleEvent(): commandId = %x, commandStatus = %x", pdu->get_commandId(), pdu->get_commandStatus());
		switch (pdu->get_commandId())
		{
			case BIND_RECIEVER_RESP:
			case BIND_TRANSMITTER_RESP:
			case BIND_TRANCIEVER_RESP:
				checkBindResp(pdu);
				break;
			case UNBIND_RESP:
				checkUnbindResp(pdu);
				setComplete(true);
				break;
			case GENERIC_NACK:
				__check__(5, !bound);
				if (allowedCmdId)
				{
					__check__(6, pdu->get_commandStatus() == ESME_RINVBNDSTS);
				}
				else if (!allowedCmdId)
				{
					__check__(7, pdu->get_commandStatus() == ESME_RINVCMDID);
				}
				else
				{
					__tc_fail__(8);
				}
				setComplete(true);
				break;
			default:
				__tc_fail__(9);
		}
	}
	virtual void handleError(int errorCode)
	{
		__trace2__("handleError(): errorCode = %d", errorCode);
		//SC не закрывает сокет при unbind
		__tc_fail__(10);
	}
};

void SmppProtocolErrorTestCases::submitAfterUnbindScenario(int num)
{
	SubmitAfterUnbindScenario scenario(cfg, smeAddr, chkList, num);
	scenario.execute();
}

class NullPduScenario : public SmppProtocolErrorScenario
{
	int num;
public:
	NullPduScenario(const SmeConfig& conf, const Address& addr,
		CheckList* chkList, int _num)
	: SmppProtocolErrorScenario(conf, addr, chkList), num(_num)
	{
		__tc__("protocolError.nullPdu");
		__trace2__("NullPduScenario(): scenario = %p", this);
	}
	~NullPduScenario()
	{
		__trace2__("~NullPduScenario(): scenario = %p", this);
	}
	virtual void execute()
	{
		//connect & bind
		connect();
		PduBindTRX bindPdu;
		sendPdu(setupBindPdu(bindPdu, BindType::Transceiver));
		//отправка пустых pdu
		for (int i = 0; i < 5; i++)
		{
			TCSelector s(num, 3);
			SmppHeader* pdu;
			switch (s.value())
			{
				case 1:
					pdu = createPdu(allowedCmdIds[rand0(allowedCmdIdsSize - 1)]);
					break;
				case 2:
					pdu = createPdu(notAllowedCmdIds[rand0(notAllowedCmdIdsSize - 1)]);
					break;
				case 3:
					pdu = createPdu(bindCmdIds[rand0(bindCmdIdsSize - 1)]);
					break;
				default:
					__unreachable__("Invalid num");
			}
			try
			{
				sendPdu(pdu);
				disposePdu(pdu);
			}
			catch (...)
			{
				__trace__("Exception when sending pdu: exiting test scenario");
				disposePdu(pdu);
				return;
			}
		}
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
		__trace2__("handleEvent(): commandId = %x, commandStatus = %x", pdu->get_commandId(), pdu->get_commandStatus());
		switch (pdu->get_commandId())
		{
			case BIND_RECIEVER_RESP:
			case BIND_TRANSMITTER_RESP:
			case BIND_TRANCIEVER_RESP:
				checkBindResp(pdu);
				break;
			case UNBIND_RESP:
				checkUnbindResp(pdu);
				setComplete(true);
				break;
			default:
				; //не проверяю
		}
	}
	virtual void handleError(int errorCode)
	{
		__trace2__("handleError(): errorCode = %d", errorCode);
		//ничего не проверяю
	}
};

void SmppProtocolErrorTestCases::nullPduScenario(int num)
{
	NullPduScenario scenario(cfg, smeAddr, chkList, num);
	scenario.execute();
}

class BindUnbindScenario : public SmppProtocolErrorScenario
{
	int num;
public:
	BindUnbindScenario(const SmeConfig& conf, const Address& addr,
		CheckList* chkList, int _num)
	: SmppProtocolErrorScenario(conf, addr, chkList), num(_num)
	{
		__trace2__("BindUnbindScenario(): scenario = %p", this);
	}
	~BindUnbindScenario()
	{
		__trace2__("~BindUnbindScenario(): scenario = %p", this);
	}
	virtual void execute()
	{
		TCSelector s(num, 3);
		//connect & bind
		connect();
		PduBindTRX bindPdu;
		switch (s.value())
		{
			case 1:
				__tc__("bind.correct.receiver");
				bindType = BindType::Receiver;
				break;
			case 2:
				__tc__("bind.correct.transmitter");
				bindType = BindType::Transmitter;
				break;
			case 3:
				__tc__("bind.correct.transceiver");
				bindType = BindType::Transceiver;
				break;
			default:
				__unreachable__("Invalid num");
		}
		sendPdu(setupBindPdu(bindPdu, bindType));
		__tc_ok__;
		//unbind
		__tc__("unbind");
		PduUnbind unbindPdu;
		sendPdu(setupUnbindPdu(unbindPdu));
		__tc_ok__;
	}
	virtual void handleEvent(SmppHeader* pdu)
	{
		__decl_tc__;
		switch (pdu->get_commandId())
		{
			case BIND_RECIEVER_RESP:
			case BIND_TRANSMITTER_RESP:
			case BIND_TRANCIEVER_RESP:
				checkBindResp(pdu);
				break;
			case UNBIND_RESP:
				checkUnbindResp(pdu);
				break;
			default:
				__unreachable__("not expected");
		}
		__tc_ok_cond__;
	}
	virtual void handleError(int errorCode)
	{
		__trace2__("handleError(): errorCode = %d", errorCode);
		__unreachable__("not expected");
	}
};

void SmppProtocolErrorTestCases::bindUnbindCorrect(int num)
{
	BindUnbindScenario scenario(cfg, smeAddr, chkList, num);
	scenario.execute();
}

}
}
}

