#include "SmeManagerTestCases.hpp"
#include "test/util/Util.cpp"
#include "test/util/TCResultFilter.hpp"
#include "test/util/CheckList.hpp"
#include "util/Logger.h"

using namespace smsc::test::util;
using namespace smsc::test::smeman; //constants, SmeManagerTestCases
using log4cpp::Category;
using smsc::util::Logger;
using smsc::smeman::SmeInfo;
using smsc::smeman::SmeProxy;
using smsc::test::util::CheckList;
static Category& log = Logger::getCategory("SmeManagerFunctionalTest");

inline void prepareForNewSme(vector<SmeInfo*>& sme, vector<TCResultStack*>& stack)
{
	sme.push_back(new SmeInfo());
	stack.push_back(new TCResultStack());
}

void executeFunctionalTest(TCResultFilter* filter, int listSize)
{
	SmeManager smeMan;
	SmeRegistry smeReg;
	SmeManagerTestCases tc(&smeMan, &smeReg);
	vector<TCResultStack*> stack;

	log.debug("*** start ***");

	//–егистраци€ sme с корректными параметрами
	for (int i = 0; i < listSize; i++)
	{
		Address smeAddr;
		SmeInfo sme;
		TCResult* res = tc.addCorrectSme(&smeAddr, &sme, RAND_TC);
		stack.push_back(new TCResultStack());
		stack.back()->push_back(res);
	}

	//–егистраци€ sme с некорректными параметрами, 1/6
	//–егистраци€ sme с пустым systemId, 1/6
	//Disable зарегистрированного sme, 1/6
	//Enable зарегистрированного sme, 1/6
	//ѕолучение зарегистрированного sme, 2/6
	bool emptySystemId = false; //можно создать только единственный sme
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
						TCResult* res = tc.addIncorrectSme(*sme);
						stack[i++]->push_back(res);
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
					TCResult* res = tc.addCorrectSmeWithEmptySystemId(&smeAddr, &sme);
					stack.push_back(new TCResultStack());
					stack.back()->push_back(res);
				}
				break;
			/*
			case 3:
				for (int i = 0; i < sme.size(); i++)
				{
					TCResult* res = tc.disableExistentSme(sme[i]);
					stack[i]->push_back(res);
				}
				break;
			case 4:
				for (int i = 0; i < sme.size(); i++)
				{
					TCResult* res = tc.enableExistentSme(sme[i]);
					stack[i]->push_back(res);
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
						TCResult* res = tc.getExistentSme(*sme, proxy);
						stack[i++]->push_back(res);
					}
					delete it;
				}
		}
	}

	//¬ыборка sme происходит равномерно
	{
		int i = 0;
		SmeRegistry::SmeIterator* it = smeReg.iterator();
		while (const SmeInfo* sme = it->next())
		{
			SmeProxy* proxy;
			TCResult* res = tc.registerCorrectSmeProxy(sme->systemId, &proxy);
			stack[i++]->push_back(res);
		}
		delete it;
	}
	
	//filter->addResult(tc.selectSme(sme, RAND_TC));

	//»терирование по списку зарегистрированных sme
	filter->addResult(tc.iterateSme());

	//”даление зарегистрированного sme
	vector<string> smeId;
	{
		int i = 0;
		SmeRegistry::SmeIterator* it = smeReg.iterator();
		while (const SmeInfo* sme = it->next())
		{
			smeId.push_back(sme->systemId);
			TCResult* res = tc.deleteExistentSme(sme->systemId);
			stack[i++]->push_back(res);
		}
		delete it;
	}

	//ѕолучение незарегистрированного/несуществующего sme
	for (int i = 0; i < smeId.size(); i++)
	{
		TCResult* res = tc.getNonExistentSme(smeId[i], RAND_TC);
		stack[i]->push_back(res);
	}

	//”даление незарегистрированного/несуществующего sme
	//Disable незарегистрированного/несуществующего sme
	//Enable незарегистрированного/несуществующего sme
	stack[0]->push_back(tc.deleteNonExistentSme());
	//stack[0]->push_back(tc.disableNonExistentSme());
	//stack[0]->push_back(tc.enableNonExistentSme());

	//»терирование по списку зарегистрированных sme
	filter->addResult(tc.iterateSme());

	//обработка результатов
	for (int i = 0; i < stack.size(); i++)
	{
		filter->addResultStack(*stack[i]);
	}

	//очистка пам€ти
	for (int i = 0; i < stack.size(); i++)
	{
		delete stack[i];
	}
}

void saveCheckList(TCResultFilter* filter)
{
	cout << "—охранение checklist" << endl;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("SME Manager", "smsc::smeman");
	//имплементированные тест кейсы
	cl.writeResult("–егистраци€ sme с корректными параметрами",
		filter->getResults(TC_ADD_CORRECT_SME));
	cl.writeResult("–егистраци€ sme с некорректными параметрами",
		filter->getResults(TC_ADD_INCORRECT_SME));
	cl.writeResult("”даление зарегистрированного sme",
		filter->getResults(TC_DELETE_EXISTENT_SME));
	cl.writeResult("”даление незарегистрированного/несуществующего sme",
		filter->getResults(TC_DELETE_NON_EXISTENT_SME));
	/*
	cl.writeResult("Disable зарегистрированного sme",
		filter->getResults(TC_DISABLE_EXISTENT_SME));
	cl.writeResult("Disable незарегистрированного/несуществующего sme",
		filter->getResults(TC_DISABLE_NON_EXISTENT_SME));
	cl.writeResult("Enable зарегистрированного sme",
		filter->getResults(TC_ENABLE_EXISTENT_SME));
	cl.writeResult("Enable незарегистрированного/несуществующего sme",
		filter->getResults(TC_ENABLE_NON_EXISTENT_SME));
	*/
	cl.writeResult("ѕолучение зарегистрированного sme",
		filter->getResults(TC_GET_EXISTENT_SME));
	cl.writeResult("ѕолучение незарегистрированного/несуществующего sme",
		filter->getResults(TC_GET_NON_EXISTENT_SME));
	cl.writeResult("»терирование по списку зарегистрированных sme",
		filter->getResults(TC_ITERATE_SME));
	cl.writeResult("¬ыборка sme происходит равномерно",
		filter->getResults(TC_SELECT_SME));
}

/**
 * ¬ыполн€ет тестирование SmeManager и
 * выводит результат по тест кейсам в checklist.
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
		TCResultFilter* filter = new TCResultFilter();
		for (int i = 0; i < numCycles; i++)
		{
			executeFunctionalTest(filter, numSme);
		}
		saveCheckList(filter);
		delete filter;
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

