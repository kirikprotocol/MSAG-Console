#include "SmeManagerTestCases.hpp"
#include "test/util/Util.cpp"
#include "SmeManagerCheckList.hpp"
#include "logger/Logger.h"

using namespace smsc::test::util;
using namespace smsc::test::smeman; //constants, SmeManagerTestCases
using smsc::logger::Logger;
using smsc::logger::Logger;
using smsc::smeman::SmeInfo;
using smsc::smeman::SmeProxy;
static Logger log = Logger::getInstance("SmeManagerFunctionalTest");

void executeFunctionalTest(int listSize, CheckList* chkList)
{
	SmeManager smeMan;
	SmeRegistry smeReg;
	SmeManagerTestCases tc(&smeMan, &smeReg, chkList);

	log.debug("*** start ***");

	//����������� sme � ����������� �����������
	for (int i = 0; i < listSize; i++)
	{
		Address smeAddr;
		SmeInfo sme;
		tc.addCorrectSme(&smeAddr, &sme, RAND_TC);
	}

	//����������� sme � ������������� �����������, 1/6
	//����������� sme � ������ systemId, 1/6
	//Disable ������������������� sme, 1/6
	//Enable ������������������� sme, 1/6
	//��������� ������������������� sme, 2/6
	bool emptySystemId = false; //����� ������� ������ ������������ sme
	for (TCSelector s(RAND_SET_TC, 6); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
			case 2:
				{
					int i = 0;
					SmeRegistry::SmeIterator* it = smeReg.iterator();
					while (const SmeInfo* sme = it->next())
					{
						tc.addIncorrectSme(*sme);
					}
					delete it;
				}
				break;
			case 3:
			case 4:
				if (!emptySystemId)
				{
					emptySystemId = true;
					Address smeAddr;
					SmeInfo sme;
					tc.addCorrectSmeWithEmptySystemId(&smeAddr, &sme);
				}
				break;
			/*
			case 3:
				for (int i = 0; i < sme.size(); i++)
				{
					tc.disableExistentSme(sme[i]);
				}
				break;
			case 4:
				for (int i = 0; i < sme.size(); i++)
				{
					tc.enableExistentSme(sme[i]);
				}
				break;
			*/
			default: //case = 5..6
				{
					int i = 0;
					SmeRegistry::SmeIterator* it = smeReg.iterator();
					while (const SmeInfo* sme = it->next())
					{
						SmeProxy* proxy;
						tc.getExistentSme(*sme, proxy);
					}
					delete it;
				}
		}
	}

	//������� sme ���������� ����������
	{
		int i = 0;
		SmeRegistry::SmeIterator* it = smeReg.iterator();
		while (const SmeInfo* sme = it->next())
		{
			SmeProxy* proxy;
			tc.registerCorrectSmeProxy(sme->systemId, &proxy);
		}
		delete it;
	}
	
	//tc.selectSme(sme, RAND_TC);

	//������������ �� ������ ������������������ sme
	tc.iterateSme();

	//�������� ������������������� sme
	vector<string> smeId;
	{
		int i = 0;
		SmeRegistry::SmeIterator* it = smeReg.iterator();
		while (const SmeInfo* sme = it->next())
		{
			smeId.push_back(sme->systemId);
			tc.deleteExistentSme(sme->systemId);
		}
		delete it;
	}

	//��������� ���������������������/��������������� sme
	for (int i = 0; i < smeId.size(); i++)
	{
		tc.getNonExistentSme(smeId[i], RAND_TC);
	}

	//�������� ���������������������/��������������� sme
	//Disable ���������������������/��������������� sme
	//Enable ���������������������/��������������� sme
	tc.deleteNonExistentSme();
	//tc.disableNonExistentSme();
	//tc.enableNonExistentSme();

	//������������ �� ������ ������������������ sme
	tc.iterateSme();
}

/**
 * ��������� ������������ SmeManager �
 * ������� ��������� �� ���� ������ � checklist.
 */
int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		cout << "Usage: RouteManagerFunctionalTest <numCycles> <numSme>" << endl;
		exit(0);
	}

	const int numCycles = atoi(argv[1]);
	const int numSme = atoi(argv[2]);
	try
	{
		//Manager::init("config.xml");
		SmeManagerCheckList chkList;
		for (int i = 0; i < numCycles; i++)
		{
			executeFunctionalTest(numSme, &chkList);
		}
		chkList.save();
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

