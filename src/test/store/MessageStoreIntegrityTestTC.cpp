#include "util/config/Manager.h"
#include "store/StoreManager.h"
#include "core/synchronization/Event.hpp"
#include "MessageStoreTestCases.hpp"
#include "test/util/CheckList.hpp"
#include "test/util/Util.hpp"
#include "test/util/TCResultFilter.hpp"
#include <iostream>

using namespace smsc::sms; //SMSId, SMS
using namespace smsc::test::store; //constants, MessageStoreTestCases
using namespace smsc::test::util; //constants, TCResult, TCResultStack, ...
using std::vector;
using std::ostringstream;
using log4cpp::Category;
using smsc::util::Logger;
using smsc::util::config::Manager;
using smsc::store::StoreManager;
using smsc::core::synchronization::Event;

static Category& log = Logger::getCategory("smsc.test.store.IntegrityTest");

#define PREPARE_FOR_NEW_SMS \
	id.push_back(new SMSId()); \
	sms.push_back(new SMS()); \
	stack.push_back(new TCResultStack());
	
void executeIntegrityTest(MessageStoreTestCases& tc,
	TCResultFilter* filter, int listSize)
{
	cout << ".";
	log.debug("*** start ***\n");
	vector<SMSId*> id;
	vector<SMS*> sms;
	vector<TCResultStack*> stack;
	static int executeTestCounter = 0;

	//���������� ����������� sms, 1/1
	for (int i = 0; i < listSize; i++)
	{
		PREPARE_FOR_NEW_SMS
		TCResult* res = tc.storeCorrectSms(id.back(), sms.back(), RAND_TC);
		stack.back()->push_back(res);
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
					PREPARE_FOR_NEW_SMS
					TCResult* res = tc.storeCorrectSms(id.back(), sms.back(),
						*id[i], *sms[i], RAND_TC);
					stack.back()->push_back(res);
				}
				break;
			case 2:
				for (int i = 0; duplicatesOk && i < listSize; i++)
				{
					PREPARE_FOR_NEW_SMS
					TCResult* res = tc.storeDuplicateSms(id.back(), sms.back(),
						*id[i], *sms[i]);
					stack.back()->push_back(res);
				}
				break;
			case 3:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.storeRejectDuplicateSms(*sms[i]);
					stack[i]->push_back(res);
				}
				break;
			case 4:
				for (int i = 0; !duplicatesOk && i < id.size(); i++)
				{
					TCResult* res = tc.storeReplaceCorrectSms(id[i], sms[i]);
					stack[i]->push_back(res);
				}
				break;
			case 5:
				{
					TCResult* res = tc.storeIncorrectSms(RAND_TC);
					filter->addResult(res);
					delete res;
					//stack[i]->push_back(res);
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
					TCResult* res = tc.changeExistentSmsStateEnrouteToEnroute(
						*id[i], sms[i], RAND_TC);
					stack[i]->push_back(res);
				}
				break;
			case 7:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.replaceCorrectSms(*id[i], sms[i], RAND_TC);
					stack[i]->push_back(res);
				}
				break;
			case 8:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.replaceIncorrectSms(*id[i], *sms[i], RAND_TC);
					stack[i]->push_back(res);
				}
				break;
			default: //case = 9..15
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.loadExistentSms(*id[i], *sms[i]);
					stack[i]->push_back(res);
				}
		}
	}

	//��������� ������ sms ��� ��������� ��������
	if (!executeTestCounter)
	{
		TCResult* res = tc.checkReadyForRetrySms(id, sms, RAND_TC);
		filter->addResult(res);
		delete res;
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
		TCResult* res = tc.changeExistentSmsStateEnrouteToFinal(*id[i], 
			sms[i], RAND_TC);
		stack[i]->push_back(res);
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
					TCResult* res = tc.changeFinalSmsStateToAny(*id[i], RAND_TC);
					stack[i]->push_back(res);
				}
				break;
			case 2:
				for (int i = 0; i < listSize; i++)
				{
					PREPARE_FOR_NEW_SMS
					TCResult* res1 = tc.storeReplaceSmsInFinalState(id.back(),
						sms.back(), *sms[i]);
					stack.back()->push_back(res1);
					//����������� ��������� ��������� ��������� � ��������� ���������
					TCResult* res2 = tc.changeExistentSmsStateEnrouteToFinal(
						*id.back(), sms.back(), RAND_TC);
					stack.back()->push_back(res2);
				}
				break;
			case 3:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.replaceFinalSms(*id[i], *sms[i]);
					stack[i]->push_back(res);
				}
				break;
			default: //4..5
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.loadExistentSms(*id[i], *sms[i]);
					stack[i]->push_back(res);
				}
		}
	}

	//��������� ������ sms ��� ��������� ��������
	if (!executeTestCounter)
	{
		TCResult* res = tc.checkReadyForRetrySms(id, sms, RAND_TC);
		filter->addResult(res);
		delete res;
	}

	//�������� ������������� sms, 1/1
	for (int i = 0; i < id.size(); i++)
	{
		TCResult* res = tc.deleteExistentSms(*id[i]);
		stack[i]->push_back(res);
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
					TCResult* res = tc.changeFinalSmsStateToAny(*id[i], RAND_TC);
					stack[i]->push_back(res);
				}
				break;
			case 2:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.replaceIncorrectSms(*id[i], *sms[i], RAND_TC);
					//TCResult* res = tc.replaceFinalSms(*id[i], *sms[i]);
					stack[i]->push_back(res);
				}
				break;
			case 3:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.loadNonExistentSms(*id[i]);
					stack[i]->push_back(res);
				}
				break;
			case 4:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.deleteNonExistentSms(*id[i]);
					stack[i]->push_back(res);
				}
				break;
		}
	}

	//��������� ������ sms ��� ��������� ��������
	if (!executeTestCounter)
	{
		TCResult* res = tc.checkReadyForRetrySms(vector<SMSId*>(),
			vector<SMS*>(), RAND_TC);
		filter->addResult(res);
		delete res;
	}

//���������� ������������� sms � ��������� �� assert
#ifdef ASSERT_THROW_IF_FAIL
	TCResult* res = tc.storeAssertSms(ALL_TC);
	filter->addResult(res);
	delete res;
#endif

	//��������� �����������
	for (int i = 0; i < stack.size(); i++)
	{
		filter->addResultStack(*stack[i]);
	}

	//������� ������
	for (int i = 0; i < id.size(); i++)
	{
		delete id[i];
		delete sms[i];
		delete stack[i];
	}
	executeTestCounter++;
}

void saveCheckList(TCResultFilter* filter)
{
	cout << "���������� checklist" << endl;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("Message Store", "smsc::store");
	//������������������ ���� �����
	cl.writeResult("���������� ����������� sms",
		filter->getResults(TC_STORE_CORRECT_SMS));
	cl.writeResult("���������� �������������� sms",
		filter->getResults(TC_STORE_DUPLICATE_SMS));
	cl.writeResult("���������� �������������� sms � �������",
		filter->getResults(TC_STORE_REJECT_DUPLICATE_SMS));
	cl.writeResult("���������� ����������� sms � ���������� ��� �������������",
		filter->getResults(TC_STORE_REPLACE_CORRECT_SMS));
	cl.writeResult("���������� sms � ���������� ������������� sms ��������� ���������",
		filter->getResults(TC_STORE_REPLACE_SMS_IN_FINAL_STATE));
	cl.writeResult("���������� ������������� sms",
		filter->getResults(TC_STORE_INCORRECT_SMS));
	cl.writeResult("���������� ������������� sms � ��������� �� assert",
		filter->getResults(TC_STORE_ASSERT_SMS));
	cl.writeResult("���������� ������� sms � ��������� ENROUTE",
		filter->getResults(TC_CHANGE_EXISTENT_SMS_STATE_ENROUTE_TO_ENROUTE));
	cl.writeResult("������� sms �� ENROUTE � ��������� ���������",
		filter->getResults(TC_CHANGE_EXISTENT_SMS_STATE_ENROUTE_TO_FINAL));
	cl.writeResult("������� ��������������� sms ��� sms � ��������� ��������� � ����� ������ ���������",
		filter->getResults(TC_CHANGE_FINAL_SMS_STATE_TO_ANY));
	cl.writeResult("���������� ���������� ������������� sms",
		filter->getResults(TC_REPLACE_CORRECT_SMS));
	cl.writeResult("������������ ���������� ������������� ��� ���������� ��������������� sms",
		filter->getResults(TC_REPLACE_INCORRECT_SMS));
	cl.writeResult("���������� ��������������� sms ��� sms � ��������� ���������",
		filter->getResults(TC_REPLACE_FINAL_SMS));
	cl.writeResult("������ ������������� sms",
		filter->getResults(TC_LOAD_EXISTENT_SMS));
	cl.writeResult("������ ��������������� sms",
		filter->getResults(TC_LOAD_NON_EXISTENT_SMS));
	cl.writeResult("�������� ������������� sms",
		filter->getResults(TC_DELETE_EXISTENT_SMS));
	cl.writeResult("�������� ��������������� sms",
		filter->getResults(TC_DELETE_NON_EXISTENT_SMS));
	cl.writeResult("��������� ������ sms ��� ��������� ��������",
		filter->getResults(TC_CHECK_READY_FOR_RETRY_SMS));

	//���� ��� ���������������������� ���� �����
	cl.writeResult("�������� ������������ SM ��������� �������� �� ������������ �����",
		filter->getResults(TC_DELETE_EXISTENT_WAITING_SM_BY_NUMBER));
	cl.writeResult("�������� �������������� SM ��������� �������� �� ������������ �����",
		filter->getResults(TC_DELETE_NON_EXISTENT_WAITING_SM_BY_NUMBER));
	cl.writeResult("�������� ��������� ������ SM ��������� �������� �� ������������ �����",
		filter->getResults(TC_LOAD_EXISTENT_WAITING_SM_BY_DESTINATION_NUMBER));
	cl.writeResult("�������� ������� ������ SM ��������� �������� �� ������������ �����",
		filter->getResults(TC_LOAD_NON_EXISTENT_WAITING_SM_BY_DESTINATION_NUMBER));
	cl.writeResult("�������� ��������� ������ SM ������������ �� ������������ �����",
		filter->getResults(TC_LOAD_EXISTENT_SM_ARCHIEVE_BY_DESTINATION_NUMBER));
	cl.writeResult("�������� ������� ������ SM ������������ �� ������������ �����",
		filter->getResults(TC_LOAD_NON_EXISTENT_SM_ARCHIEVE_BY_DESTINATION_NUMBER));
	cl.writeResult("�������� ��������� ������ SM ������������ � ������������� ������",
		filter->getResults(TC_LOAD_EXISTENT_SM_ARCHIEVE_BY_ORIGINATING_NUMBER));
	cl.writeResult("�������� ������� ������ SM ������������ � ������������� ������",
		filter->getResults(TC_LOAD_NON_EXISTENT_SM_ARCHIEVE_BY_ORIGINATING_NUMBER));
	cl.writeResult("�������� �������� ���������� ������ ���������� ���������",
		filter->getResults(TC_GET_EXISTENT_SM_DELIVERY_FAILURE_STATISTICS));
	cl.writeResult("�������� ������ ���������� ������ ���������� ���������",
		filter->getResults(TC_GET_NON_EXISTENT_SM_DELIVERY_FAILURE_STATISTICS));
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
		TCResultFilter* filter = new TCResultFilter();
		MessageStoreTestCases tc(StoreManager::getMessageStore()); //throws exception
		for (int i = 0; i < numCycles; i++)
		{
			executeIntegrityTest(tc, filter, numSms);
		}
		saveCheckList(filter);
		delete filter;
		StoreManager::shutdown();
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

