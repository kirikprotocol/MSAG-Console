#include "SmppBaseTestCases.hpp"
#include "test/conf/TestConfig.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/Util.hpp"

namespace smsc {
namespace test {
namespace sme {

using smsc::util::Logger;
using smsc::core::synchronization::MutexGuard;
using smsc::test::conf::TestConfig;
using namespace smsc::test::core; //flags
using namespace smsc::test::util;
using namespace smsc::smpp::SmppCommandSet;

Category& SmppBaseTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmppBaseTestCases");
	return log;
}

bool SmppBaseTestCases::bindCorrectSme(int num)
{
	TCSelector s(num, 1);
	__decl_tc__;
	for (; s.check(); s++)
	{
		try
		{
			switch(s.value())
			{
				case 1: //BIND_TRANCIEVER
					__tc__("bindCorrectSme.bindTransceiver");
					break;
				/*
				case 2: //BIND_RECIEVER
					__tc__("bindCorrectSme.bindReceiver");
					__tc_fail__(1);
					break;
				case 3: //BIND_TRANSMITTER
					__tc__("bindCorrectSme.bindTransmitter");
					__tc_fail__(2);
					break;
				*/
				default:
					__unreachable__("Invalid num");
			}
			fixture->session->connect();
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

void SmppBaseTestCases::bindIncorrectSme(int num)
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
						try
						{
							SmeConfig conf(config);
							auto_ptr<char> tmp = rand_char(15); //15 по спецификации
							conf.sid = tmp.get();
							FakeReceiver receiver;
							SmppSession sess(conf, &receiver);
							sess.connect();
						}
						catch(...)
						{
							throw;
						}
					}
					break;
				case 2: //bind на недоступный SC (неизвестный хост)
					{
						__tc__("bindIncorrectSme.unknownHost");
						try
						{
							SmeConfig conf(config);
							auto_ptr<char> tmp = rand_char(15);
							conf.host = tmp.get();
							FakeReceiver receiver;
							SmppSession sess(conf, &receiver);
							sess.connect();
						}
						catch(...)
						{
							throw;
						}
					}
					break;
				case 3: //bind на недоступный SC (неправильный порт)
					{
						__tc__("bindIncorrectSme.invalidPort");
						try
						{
							SmeConfig conf(config);
							auto_ptr<char> tmp = rand_char(15);
							conf.port += rand1(65535 - conf.port);
							FakeReceiver receiver;
							SmppSession sess(conf, &receiver);
							sess.connect();
						}
						catch(...)
						{
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

void SmppBaseTestCases::checkMissingPdu(time_t checkTime)
{
	__decl_tc__;
	//проверить неполученные доставки и подтверждения доставки
	//по окончании validTime
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
				//break;
			default:
				__unreachable__("Invalid monitor type");
		}
		fixture->pduReg->removeMonitor(monitor);
		//респонсы считаются валидными всегда
		if (monitor->pduData->valid || monitor->getType() == RESPONSE_MONITOR)
		{
			switch (monitor->getFlag())
			{
				case PDU_REQUIRED_FLAG:
					__tc_fail__(1);
					monitor->setMissingOnTime();
					break;
				case PDU_MISSING_ON_TIME_FLAG:
				case PDU_RECEIVED_FLAG:
				case PDU_NOT_EXPECTED_FLAG:
				case PDU_EXPIRED_FLAG:
					__require__(monitor->getCheckTime() == monitor->getValidTime());
					break;
				default:
					__unreachable__("Invalid flag");
			}
		}
		else
		{
			switch (monitor->getFlag())
			{
				case PDU_REQUIRED_FLAG:
				case PDU_MISSING_ON_TIME_FLAG:
					monitor->setNotExpected();
					break;
				case PDU_RECEIVED_FLAG:
					__tc_fail__(2);
					//break;
				case PDU_NOT_EXPECTED_FLAG:
				case PDU_EXPIRED_FLAG:
					__require__(monitor->getCheckTime() == monitor->getValidTime());
					break;
				default:
					__unreachable__("Invalid flag");
			}
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

