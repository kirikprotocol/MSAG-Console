#include "util/config/Manager.h"
#include "store/StoreManager.h"
#include "core/synchronization/Event.hpp"
#include "MessageStoreTestCases.hpp"
#include "MessageStoreCheckList.hpp"
#include "test/util/Util.hpp"
#include <iostream>

using namespace smsc::sms; //SMSId, SMS
using namespace smsc::test::store; //constants, MessageStoreTestCases
using namespace smsc::test::util; //constants, ...
using std::vector;
using std::ostringstream;
using log4cpp::Category;
using smsc::util::Logger;
using smsc::util::config::Manager;
using smsc::store::StoreManager;
using smsc::core::synchronization::Event;

static Category& log = Logger::getCategory("smsc.test.store.IntegrityTest");

#define __prepare_for_new_sms__ \
	id.push_back(new SMSId()); \
	sms.push_back(new SMS());
	
void executeIntegrityTest(MessageStoreTestCases& tc, int listSize)
{
	cout << ".";
	log.debug("*** start ***\n");
	vector<SMSId*> id;
	vector<SMS*> sms;
	static int executeTestCounter = 0;

	//���������� ����������� sms, 1/1
	for (int i = 0; i < listSize; i++)
	{
		__prepare_for_new_sms__
		tc.storeCorrectSms(id.back(), sms.back(), RAND_TC);
	}
	
	//���������� ����������� sms � ����������� �������� �� ��� ������������ sms, 1/15
	//���������� �������������� sms, 1/15
	//���������� �������������� sms � �������, 1/15
	//���������� ����������� sms � ���������� ��� �������������� 1/15
	//���������� ������������� sms, 1/15
	//���������� ������� sms � ��������� ENROUTE, 1/15
	//���������� ���������� ������������� sms, 1/15
    //������������ ���������� ������������� sms, 1/15
	//������ ������������� sms, 7/15
	bool duplicatesOk = rand0(1); //����������������� ���� �����
	for (TCSelector s(RAND_SET_TC, 15); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				for (int i = 0; i < listSize; i++)
				{
					__prepare_for_new_sms__
					tc.storeSimilarSms(id.back(), sms.back(),
						*id[i], *sms[i], RAND_TC);
				}
				break;
			case 2:
				for (int i = 0; duplicatesOk && i < listSize; i++)
				{
					__prepare_for_new_sms__
					tc.storeDuplicateSms(id.back(), sms.back(),
						*id[i], *sms[i]);
				}
				break;
			case 3:
				for (int i = 0; i < id.size(); i++)
				{
					tc.storeRejectDuplicateSms(*sms[i]);
				}
				break;
			case 4:
				for (int i = 0; !duplicatesOk && i < id.size(); i++)
				{
					tc.storeReplaceCorrectSms(id[i], sms[i]);
				}
				break;
			case 5:
				{
					tc.storeIncorrectSms(RAND_TC);
				}
				break;
			case 6:
				//�������� �����, ����� SMS::lastTime ���������� �� �����������
				if (!rand0(3))
				{
					static Event evt;
					evt.Wait(1100);
					log.debug("*** wait ***\n");
				}
				for (int i = 0; i < id.size(); i++)
				{
					tc.changeExistentSmsStateEnrouteToEnroute(
						*id[i], sms[i], RAND_TC);
				}
				break;
			case 7:
				for (int i = 0; i < id.size(); i++)
				{
					tc.replaceCorrectSms(*id[i], sms[i], RAND_TC);
				}
				break;
			case 8:
				for (int i = 0; i < id.size(); i++)
				{
					tc.replaceIncorrectSms(*id[i], *sms[i], RAND_TC);
				}
				break;
			default: //case = 9..15
				for (int i = 0; i < id.size(); i++)
				{
					tc.loadExistentSms(*id[i], *sms[i]);
				}
		}
	}

	//��������� ������ sms ��� ��������� ��������
	if (!executeTestCounter)
	{
		tc.checkReadyForRetrySms(id, sms, RAND_TC);
	}

	//�������� �����, ����� SMS::lastTime ���������� �� �����������
	if (!rand0(3))
	{
		static Event evt;
		evt.Wait(1100);
		log.debug("*** wait ***\n");
	}
	//������� sms �� ENROUTE � ��������� ���������, 1/1
	for (int i = 0; i < id.size(); i++)
	{
		tc.changeExistentSmsStateEnrouteToFinal(*id[i], 
			sms[i], RAND_TC);
	}
	
	//������� sms � ��������� ��������� � ����� ������ ���������, 1/5
	//���������� sms � ���������� ������������� sms ��������� ���������, 1/5
	//���������� sms � ��������� ���������, 1/5
	//������ ������������� sms, 2/5
	for (TCSelector s(RAND_SET_TC, 5); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				for (int i = 0; i < id.size(); i++)
				{
					tc.changeFinalSmsStateToAny(*id[i], RAND_TC);
				}
				break;
			case 2:
				for (int i = 0; i < listSize; i++)
				{
					__prepare_for_new_sms__
					tc.storeReplaceSmsInFinalState(id.back(), sms.back(), *sms[i]);
					//����������� ��������� ��������� ��������� � ��������� ���������
					tc.changeExistentSmsStateEnrouteToFinal(*id.back(),
						sms.back(), RAND_TC);
				}
				break;
			case 3:
				for (int i = 0; i < id.size(); i++)
				{
					tc.replaceFinalSms(*id[i], *sms[i]);
				}
				break;
			default: //4..5
				for (int i = 0; i < id.size(); i++)
				{
					tc.loadExistentSms(*id[i], *sms[i]);
				}
		}
	}

	//��������� ������ sms ��� ��������� ��������
	if (!executeTestCounter)
	{
		tc.checkReadyForRetrySms(id, sms, RAND_TC);
	}

	//�������� ������������� sms, 1/1
	for (int i = 0; i < id.size(); i++)
	{
		tc.deleteExistentSms(*id[i]);
	}

	//������� ��������������� sms � ����� ���������, 1/4
	//������������ ���������� ��������������� sms, 1/4
	//������ ��������������� sms, 1/4
	//�������� ��������������� sms, 1/4
	for (TCSelector s(RAND_SET_TC, 4); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				for (int i = 0; i < id.size(); i++)
				{
					tc.changeFinalSmsStateToAny(*id[i], RAND_TC);
				}
				break;
			case 2:
				for (int i = 0; i < id.size(); i++)
				{
					tc.replaceIncorrectSms(*id[i], *sms[i], RAND_TC);
					//tc.replaceFinalSms(*id[i], *sms[i]);
				}
				break;
			case 3:
				for (int i = 0; i < id.size(); i++)
				{
					tc.loadNonExistentSms(*id[i]);
				}
				break;
			case 4:
				for (int i = 0; i < id.size(); i++)
				{
					tc.deleteNonExistentSms(*id[i]);
				}
				break;
		}
	}

	//��������� ������ sms ��� ��������� ��������
	if (!executeTestCounter)
	{
		tc.checkReadyForRetrySms(vector<SMSId*>(),
			vector<SMS*>(), RAND_TC);
	}

//���������� ������������� sms � ��������� �� assert
#ifdef ASSERT_THROW_IF_FAIL
	tc.storeAssertSms(ALL_TC);
#endif

	//������� ������
	for (int i = 0; i < id.size(); i++)
	{
		delete id[i];
		delete sms[i];
	}
	executeTestCounter++;
}

/**
 * ��������� ������������ ����������� ������ (Data and Database Integrity 
 * Testing) � ������� ��������� �� ���� ������ � checklist.
 */
int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		cout << "Usage: MessageStoreIntegrityTestTC <numCycles> <numSms>" << endl;
		exit(0);
	}

	const int numCycles = atoi(argv[1]);
	const int numSms = atoi(argv[2]);
	try
	{
		Manager::init("config.xml");
		StoreManager::startup(Manager::getInstance());
		//StoreManager::stopArchiver();
		MessageStoreCheckList chkList;
		MessageStoreTestCases tc(StoreManager::getMessageStore(), false,
			&chkList); //throws exception
		for (int i = 0; i < numCycles; i++)
		{
			executeIntegrityTest(tc, numSms);
		}
		cout << endl;
		StoreManager::shutdown();
		chkList.save();
		chkList.saveHtml();
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

