#include "RouteManagerTestCases.hpp"
#include "test/smeman/SmeManagerTestCases.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/util/TCResultFilter.hpp"
#include "test/util/CheckList.hpp"
#include "util/Logger.h"

using namespace smsc::test::util;
using namespace smsc::test::smeman; //constants, SmeManagerTestCases
using namespace smsc::test::router; //constants, TestRouteData
using log4cpp::Category;
using smsc::sms::Address;
using smsc::util::Logger;
using smsc::smeman::SmeInfo;
using smsc::smeman::SmeProxy;
using smsc::test::sms::SmsUtil;
using smsc::test::util::CheckList;

static Category& log = Logger::getCategory("RouteManagerFunctionalTest");

void prepareForNewRoute(const Address& origAddr, const Address& destAddr,
	vector<SmeInfo*>& sme, vector<TestRouteData*>& routeData,
	vector<TCResultStack*>& stack, SmeManagerTestCases& tcSme)
{
	routeData.push_back(new TestRouteData(origAddr, destAddr));
	//��� ������� �������� - ��������� sme (��� �������������)
	sme.push_back(new SmeInfo());
	TCResult* res1 = tcSme.addCorrectSme(sme.back(), RAND_TC);
	TCResult* res2 = tcSme.registerCorrectSmeProxy(sme.back()->systemId);
	stack.back()->push_back(res1);
	stack.back()->push_back(res2);
}

void executeTestCases(const Address& origAddr, const Address& destAddr,
	vector<SmeInfo*>& sme, vector<TestRouteData*>& routeData,
	vector<TCResultStack*>& stack, SmeManagerTestCases& tcSme,
	RouteManagerTestCases& tcRoute)
{
	log.debug("*** start ***");

	//�������� ������ ����� ��� origAddr, destAddr
	stack.push_back(new TCResultStack());

	//���������� ����������� ��������
	prepareForNewRoute(origAddr, destAddr, sme, routeData, stack, tcSme);
	TCResult* res = tcRoute.addCorrectRoute(sme.back()->systemId,
		routeData.back(), RAND_TC);
	stack.back()->push_back(res);

	//���������� ����������� ��������
	//���������� ����������� �������� � ������������� (��������������) ����������
	//���������� ������������� ��������
	//����� ��������
	for (TCSelector s(RAND_SET_TC, 5); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				{
					prepareForNewRoute(origAddr, destAddr, sme, routeData, stack, tcSme);
					TCResult* res = tcRoute.addCorrectRoute(
						sme.back()->systemId, routeData.back(), RAND_TC);
					stack.back()->push_back(res);
				}
				break;
			case 2:
				{
					prepareForNewRoute(origAddr, destAddr, sme, routeData, stack, tcSme);
					TCResult* res = tcRoute.addCorrectRoute2(
						sme.back()->systemId, routeData.back(), RAND_TC);
					stack.back()->push_back(res);
				}
				break;
			case 3:
				{
					TCResult* res = tcRoute.addIncorrectRoute(
						sme.back()->systemId, *routeData.back()->route, RAND_TC);
					stack.back()->push_back(res);
				}
				break;
			default: //case 4..5
				{
					TCResult* res = tcRoute.lookupRoute(origAddr, destAddr, routeData);
					stack.back()->push_back(res);
				}
		}
	}
}

void executeFunctionalTest(TCResultFilter* filter, int numAddr)
{
	SmeManagerTestCases tcSme;
	RouteManagerTestCases tcRoute;
	vector<Address*> addr;
	vector<SmeInfo*> sme;
	vector<TestRouteData*> routeData;
	vector<TCResultStack*> stack;

	//���������� ������ �������
	for (int i = 0; i < numAddr; i++)
	{
		addr.push_back(new Address());
		SmsUtil::setupRandomCorrectAddress(addr.back());
	}

	//�� ������ ���� ������� ����������� ���������� ���������� ���������
	for (int i = 0; i < numAddr; i++)
	{
		for (int j = 0; j < numAddr; j++)
		{
			Address& origAddr = *addr[i];
			Address& destAddr = *addr[j];
			executeTestCases(origAddr, destAddr, sme, routeData,
				stack, tcSme, tcRoute);
		}
	}

	//����� �������� ��� ������ ���� �������
	for (int i = 0; i < numAddr; i++)
	{
		for (int j = 0; j < numAddr; j++)
		{
			Address& origAddr = *addr[i];
			Address& destAddr = *addr[j];
			TCResult* res = tcRoute.lookupRoute(origAddr, destAddr, routeData);
			filter->addResult(res);
		}
	}

	//������������ �� ������ ���������
	filter->addResult(tcRoute.iterateRoutes(routeData));

	//�������� ������������������ sme
	for (int i = 0; i < sme.size(); i++)
	{
		TCResult* res = tcSme.deleteExistentSme(sme[i]->systemId);
		filter->addResult(res);
	}

	//����� �������� ��� ������ ���� �������
	for (int i = 0; i < numAddr; i++)
	{
		for (int j = 0; j < numAddr; j++)
		{
			Address& origAddr = *addr[i];
			Address& destAddr = *addr[j];
			TCResult* res = tcRoute.lookupRoute(origAddr, destAddr, routeData);
			filter->addResult(res);
		}
	}

	//������������ �� ������ ���������
	filter->addResult(tcRoute.iterateRoutes(routeData));

	//��������� �����������
	for (int i = 0; i < stack.size(); i++)
	{
		filter->addResultStack(*stack[i]);
	}

	//������� ������
	for (int i = 0; i < sme.size(); i++)
	{
smsc::sms::AddressValue oa, od, oa2, od2;
routeData[i]->origAddr.getValue(oa);
routeData[i]->destAddr.getValue(od);
routeData[i]->route->source.getValue(oa2);
routeData[i]->route->source.getValue(od2);
log.debug("%s\t%s\t(%s\t%s), match = %d, origAddrMatch = %f, destAddrMatch = %f",
	oa, od, oa2, od2, routeData[i]->match,
	routeData[i]->origAddrMatch, routeData[i]->destAddrMatch);
		delete sme[i];
		delete routeData[i];
	}
	for (int i  = 0; i < addr.size(); i++)
	{
		delete addr[i];
	}
	for (int i = 0; i < stack.size(); i++)
	{
		delete stack[i];
	}
}

void saveCheckList(TCResultFilter* filter)
{
	cout << "���������� checklist" << endl;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("Route Manager", "smsc::router");
	//������������������ ���� �����
	cl.writeResult("���������� ����������� ��������",
		filter->getResults(TC_ADD_CORRECT_ROUTE));
	cl.writeResult("���������� ������������� ��������",
		filter->getResults(TC_ADD_INCORRECT_ROUTE));
	cl.writeResult("����� ��������",
		filter->getResults(TC_LOOKUP_ROUTE));
	cl.writeResult("������������ �� ������ ���������",
		filter->getResults(TC_ITERATE_ROUTES));
}

/**
 * ��������� ������������ RouteManager �
 * ������� ��������� �� ���� ������ � checklist.
 */
int main(int argc, char* argv[])
{
	try
	{
		//Manager::init("config.xml");
		TCResultFilter* filter = new TCResultFilter();
		executeFunctionalTest(filter, 100);
		saveCheckList(filter);
		delete filter;
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

