#ifndef TEST_CONFIG_CONFIG_GEN_CHECK_LIST
#define TEST_CONFIG_CONFIG_GEN_CHECK_LIST

#include "test/util/CheckList.hpp"

namespace smsc {
namespace test {
namespace config {

using smsc::test::util::CheckList;

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
class ConfigGenCheckList : public CheckList
{
public:
	ConfigGenCheckList()
		: CheckList("���������� ��������������� ������������ ������������ SC", "config.chk")
	{
		//aliasConfig
		__reg_tc__("aliasConfig",
			"������������ alias manager (aliases.xml)");
		__reg_tc__("aliasConfig.correctAlias",
			"���������� ����� (������ ������������ ����������� � ������������ �������� ������������)");
		__reg_tc__("aliasConfig.incorrectAlias",
			"������������ ����� ...........");
		//smeConfig
		__reg_tc__("smeConfig",
			"������������ sme manager (sme.xml)");
		__reg_tc__("smeConfig.correctSme",
			"���������� sme");
		__reg_tc__("smeConfig.incorrectSme",
			"������������ sme ...........");
		//routeConfig
		__reg_tc__("routeConfig",
			"������������ route manager (routes.xml)");
		__reg_tc__("routeConfig.billing",
			"�������� � ���������� ���������");
		__reg_tc__("routeConfig.archivate",
			"�������� � ���������� ���������� sms");
		__reg_tc__("routeConfig.notEnabling",
			"����������� ��������");
		//routeConfig.routeSource
		__reg_tc__("routeConfig.routeSource",
			"������� ����������� (��� <source>) � ��������");
		__reg_tc__("routeConfig.routeSource.subject",
			"������������� ��������� ��� ������� �����������");
		__reg_tc__("routeConfig.routeSource.subject.fakeMaskBefore",
			"� �������� ����� ����������� ������ ������ ����������� ���� ������ ����� (��� <mask>)");
		__reg_tc__("routeConfig.routeSource.subject.fakeMaskAfter",
			"� �������� ����� ����������� ����� ������ ����������� ���� ������ ����� (��� <mask>)");
		__reg_tc__("routeConfig.routeSource.mask",
			"������������� ����� ������ ��� ������� �����������");
		__reg_tc__("routeConfig.routeSource.fakeSourceBefore",
			"� �������� ����� ����������� ������������ (��� <source>) ���� ������ �����������");
		__reg_tc__("routeConfig.routeSource.fakeSourceAfter",
			"� �������� ����� ������������ ����������� (��� <source>) ���� ������ �����������");
		//routeConfig.routeDest
		__reg_tc__("routeConfig.routeDest",
			"������� ���������� (��� <dest>) � ��������");
		__reg_tc__("routeConfig.routeDest.subject",
			"������������� ��������� ��� ������� ����������");
		__reg_tc__("routeConfig.routeDest.subject.fakeMaskBefore",
			"� �������� ����� ����������� ������ ������ ���������� ���� ������ ����� (��� <mask>)");
		__reg_tc__("routeConfig.routeDest.subject.fakeMaskAfter",
			"� �������� ����� ����������� ����� ������ ���������� ���� ������ ����� (��� <mask>)");
		__reg_tc__("routeConfig.routeDest.mask",
			"������������� ����� ������ ��� ������� ����������");
		__reg_tc__("routeConfig.routeDest.fakeDestBefore",
			"� �������� ����� ����������� ����������� (��� <dest>) ���� ������ ����������");
		__reg_tc__("routeConfig.routeDest.fakeDestAfter",
			"� �������� ����� ������������ ���������� (��� <dest>) ���� ������ ����������");
		//routeConfig.incorrectRoute
		__reg_tc__("routeConfig.incorrectRoute",
			"������������ ������� ...........");
	}
};

}
}
}

#endif /* TEST_CONFIG_CONFIG_GEN_CHECK_LIST */
