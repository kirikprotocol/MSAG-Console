#include "SmppBaseTestCases.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/smpp/TestSmppSession.hpp"
#include "test/util/Util.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::core::synchronization::MutexGuard;
using smsc::test::conf::TestConfig;
using namespace smsc::sme; //SmppConnectException, BindType
using namespace smsc::smpp::SmppCommandSet;
using namespace smsc::smpp::SmppStatusSet;
using namespace smsc::test::core; //flags
using namespace smsc::test::smpp;
using namespace smsc::test::util;

Category& SmppBaseTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppBaseTestCases");
	return log;
}

bool SmppBaseTestCases::bindCorrectSme()
{
	__decl_tc__;
	try
	{
		int bindType;
		switch(fixture->smeType)
		{
			case SME_TRANSCEIVER:
				__tc__("bind.correct.transceiver");
				bindType = BindType::Transceiver;
				break;
			case SME_RECEIVER:
				__tc__("bind.correct.receiver");
				bindType = BindType::Receiver;
				break;
			case SME_TRANSMITTER:
				__tc__("bind.correct.transmitter");
				bindType = BindType::Transmitter;
				break;
			default:
				__unreachable__("Invalid smeType");
		}
		fixture->session->connect(bindType);
		__tc_ok_cond__;
		return true;
	}
	catch (SmppConnectException& e)
	{
		__tc_fail__(100);
		__trace2__("Bind failed: %s for systemId = %s, password = %s",
			e.getTextReason(), fixture->smeInfo.systemId.c_str(), fixture->smeInfo.password.c_str());
		return false;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
		return false;
	}
}

void SmppBaseTestCases::bindIncorrectSme(int num)
{
	__trace__("bindIncorrectSme(): before");
	//��������� bind ����� �� ���� ������, �.�. �������� � ������������ ��������
	//��� ������
	TCSelector s(num, 4);
	__decl_tc__;
	for (; s.check(); s++)
	{
		int reason;
		try
		{
			char tmp[64];
			SmeConfig conf(config);
			switch(s.value())
			{
				case 1: //sme �� ���������������� � SC
					__tc__("bind.incorrect.systemIdNotRegistered");
					reason = SmppConnectException::Reason::bindFailed;
					rand_char(15, tmp); //15 �� ������������
					conf.sid = tmp;
					break;
				case 2: //������������ ������
					__tc__("bind.incorrect.invalidPassword");
					reason = SmppConnectException::Reason::bindFailed;
					rand_char(8, tmp); //8 �� ������������
					conf.password = tmp;
					break;
				case 3: //bind �� ����������� SC (����������� ����)
					__tc__("bind.incorrect.unknownHost");
					reason = SmppConnectException::Reason::networkResolve;
					rand_char(15, tmp);
					conf.host = tmp;
					break;
				case 4: //bind �� ����������� SC (������������ ����)
					__tc__("bind.incorrect.invalidPort");
					reason = SmppConnectException::Reason::networkConnect;
					conf.port += rand1(65535 - conf.port);
					break;
				/*
				case 5: //������������ ����� ���� systemId
					__tc__("bind.incorrect.invalidSystemIdLength");
					reason = SmppConnectException::Reason::networkConnect;
					rand_char(16, tmp); //15 �� ������������
					conf.sid = tmp;
					break;
				case 6: //������������ ����� ���� password
					__tc__("bind.incorrect.invalidPasswordLength");
					reason = SmppConnectException::Reason::networkConnect;
					rand_char(9, tmp);  //8 �� ������������
					conf.password = tmp;
					break;
				case 7: //������������ ����� ���� systemType
					__tc__("bind.incorrect.invalidSystemTypeLength");
					reason = SmppConnectException::Reason::networkConnect;
					rand_char(13, tmp);  //12 �� ������������
					conf.systemType = tmp;
					break;
				case 8: //������������ ����� ���� addressRange
					__tc__("bind.incorrect.invalidAddressRangeLength");
					reason = SmppConnectException::Reason::networkConnect;
					rand_char(41, tmp);  //40 �� ������������
					conf.addressRange = tmp;
					break;
				*/
				default:
					__unreachable__("Invalid num");
			}
			FakeReceiver receiver;
			SmppSession sess(conf, &receiver);
			sess.connect();
			__tc_fail__(1);
		}
		catch(SmppConnectException& e)
		{
			if (e.getReason() != reason)
			{
				__tc_fail__(2);
				__trace2__("reason = %d, expected = %d", e.getReason(), reason);
			}
			__tc_ok_cond__;
		}
		catch(...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

#define __check__(errCode, cond) \
	if (!(cond)) { __tc_fail__(errCode); }

class BindUnbindListener : public SmppPduEventListener
{
	uint32_t cmdId;
	bool bound;
	CheckList* chkList;
public:
	BindUnbindListener(uint32_t _cmdId, CheckList* _chkList)
	: cmdId(_cmdId), chkList(_chkList), bound(false) {}
	void checkBindResp(SmppHeader* pdu)
	{
		__decl_tc__;
		__tc__("bind.resp.checkCommandStatus");
		__check__(1, pdu->get_commandStatus() == ESME_ROK);
		__tc_ok_cond__;
		__tc__("bind.resp.checkInterfaceVersion");
		__check__(1, reinterpret_cast<PduBindTRXResp*>(pdu)->get_scInterfaceVersion() == 0x34);
		__tc_ok_cond__;
	}
	void checkUnbindResp(SmppHeader* pdu)
	{
		__decl_tc__;
		__tc__("unbind.resp.checkCommandStatus");
		__check__(1, pdu->get_commandStatus() == ESME_ROK);
		__tc_ok_cond__;
	}
	virtual void handleEvent(SmppHeader* pdu)
	{
		__decl_tc__;
		switch (pdu->get_commandId())
		{
			case BIND_RECIEVER_RESP:
				__tc__("bind.resp.receiver");
				__check__(1, cmdId == BIND_RECIEVER);
				__check__(2, !bound);
				checkBindResp(pdu);
				bound = true;
				break;
			case BIND_TRANSMITTER_RESP:
				__tc__("bind.resp.transmitter");
				__check__(1, cmdId == BIND_TRANSMITTER);
				__check__(2, !bound);
				checkBindResp(pdu);
				bound = true;
				break;
			case BIND_TRANCIEVER_RESP:
				__tc__("bind.resp.transceiver");
				__check__(1, cmdId == BIND_TRANCIEVER);
				__check__(2, !bound);
				checkBindResp(pdu);
				bound = true;
				break;
			case UNBIND_RESP:
				__tc__("unbind.resp");
				__check__(1, bound);
				checkUnbindResp(pdu);
				bound = false;
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

void SmppBaseTestCases::bindUnbindCorrect(int num)
{
	__decl_tc__;
	TCSelector s(num, 3);
	for (; s.check(); s++)
	{
		try
		{
			__tc__("bind.correct");
			uint32_t cmdId;
			switch (s.value())
			{
				case 1:
					__tc__("bind.correct.transceiver");
					cmdId = BIND_TRANCIEVER;
					break;
				case 2:
					__tc__("bind.correct.receiver");
					cmdId = BIND_RECIEVER;
					break;
				case 3:
					__tc__("bind.correct.transmitter");
					cmdId = BIND_TRANSMITTER;
					break;
				default:
					__unreachable__("Invalid num");
			}
			BindUnbindListener listener(cmdId, chkList);
			TestSmppSession sess(&listener);
			sess.connect(config.host.c_str(), config.port, config.timeOut);
			//bind
			PduBindTRX bindPdu;
			bindPdu.get_header().set_commandId(cmdId);
			bindPdu.set_systemId(config.sid.c_str());
			bindPdu.set_password(config.password.c_str());
			bindPdu.set_systemType(config.systemType.c_str());
			bindPdu.get_header().set_sequenceNumber(sess.getNextSeq());
			sess.sendPdu(reinterpret_cast<SmppHeader*>(&bindPdu));
			__tc_ok__;
			//unbind
			__tc__("unbind");
			PduUnbind unbindPdu;
			unbindPdu.get_header().set_commandId(SmppCommandSet::UNBIND);
			unbindPdu.get_header().set_sequenceNumber(sess.getNextSeq());
			sess.sendPdu(reinterpret_cast<SmppHeader*>(&unbindPdu));
			sess.close();
			__tc_ok__;
		}
		catch (...)
		{
			__tc_fail__(100);
			error();
		}
	}
}

void SmppBaseTestCases::checkMissingPdu(time_t checkTime)
{
	__decl_tc__;
	//��������� ������������ �������� � ������������� ��������
	//�� ��������� validTime
	PduRegistry::PduMonitorIterator* it = fixture->pduReg->getMonitors(0, checkTime);
	int found = 0;
	int moved = 0;
	int deleted = 0;
	while (PduMonitor* monitor = it->next())
	{
		found++;
		switch (monitor->getType())
		{
			case RESPONSE_MONITOR:
				__tc__("checkMissingPdu.response");
				break;
			case DELIVERY_MONITOR:
				__tc__("checkMissingPdu.delivery");
				break;
			case DELIVERY_RECEIPT_MONITOR:
				__tc__("checkMissingPdu.deliveryReceipt");
				break;
			case SME_ACK_MONITOR:
				__tc__("checkMissingPdu.smeAcknoledgement");
				break;
			case INTERMEDIATE_NOTIFICATION_MONITOR:
				__tc__("checkMissingPdu.intermediateNotification");
				break;
			case GENERIC_NACK_MONITOR:
				__tc__("checkMissingPdu.genericNack");
				break;
			default:
				__unreachable__("Invalid monitor type");
		}
		fixture->pduReg->removeMonitor(monitor);
		switch (monitor->getFlag())
		{
			case PDU_REQUIRED_FLAG:
				__tc_fail__(1);
				monitor->setMissingOnTime();
				break;
			case PDU_COND_REQUIRED_FLAG:
				//ok
				break;
			case PDU_MISSING_ON_TIME_FLAG:
			case PDU_NOT_EXPECTED_FLAG:
				__require__(monitor->getCheckTime() == monitor->getValidTime());
				break;
			default:
				__unreachable__("Invalid flag");
		}
		__tc_ok_cond__;
		if (monitor->getValidTime() < checkTime)
		{
			deleted++;
			delete monitor;
		}
		else
		{
			moved++;
			fixture->pduReg->registerMonitor(monitor);
		}
	}
	delete it;
	__trace2__("checkMissingPdu(): found = %d, moved = %d, deleted = %d", found, moved, deleted);
}

void SmppBaseTestCases::checkMissingPdu()
{
	if (fixture->pduReg)
	{
		MutexGuard mguard(fixture->pduReg->getMutex());
		__cfg_int__(timeCheckAccuracy);
		time_t checkTime = time(NULL) - timeCheckAccuracy;
		checkMissingPdu(checkTime);
		__trace2__("checkMissingPdu(): pduReg size = %d", fixture->pduReg->size());
	}
}

void SmppBaseTestCases::unbind()
{
	__decl_tc__;
	__tc__("unbind");
	try
	{
		fixture->session->close();
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

