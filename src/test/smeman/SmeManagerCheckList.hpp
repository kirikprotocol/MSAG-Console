#ifndef TEST_SMEMAN_SME_MANAGER_CHECK_LIST
#define TEST_SMEMAN_SME_MANAGER_CHECK_LIST

#include "test/util/CheckList.hpp"

namespace smsc {
namespace test {
namespace smeman {

using smsc::test::util::CheckList;

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
class SmeManagerCheckList : public CheckList
{
public:
	SmeManagerCheckList()
		: CheckList("���������� ��������������� ������������ Sme Manager", "sme_man.chk")
	{
		//addCorrectSme
		__reg_tc__("addCorrectSme", "����������� sme � ����������� �����������");
		__reg_tc__("addCorrectSme.invalidType",
			"�������� typeOfNumber ��� ��������� ������������ SMPP v3.4");
		__reg_tc__("addCorrectSme.invalidPlan",
			"�������� numberingPlan ��� ��������� ������������ SMPP v3.4");
		__reg_tc__("addCorrectSme.invalidVersion",
			"�������� interfaceVersion ��� ��������� ������������ SMPP v3.4");
		__reg_tc__("addCorrectSme.invalidAddressRangeLength",
			"������������ �������� ����� ���� addressRange");
		__reg_tc__("addCorrectSme.invalidSystemTypeLength",
			"������������ �������� ����� ���� systemType");
		__reg_tc__("addCorrectSme.invalidPasswordLength",
			"������������ �������� ����� ���� password");
		__reg_tc__("addCorrectSme.invalidSystemIdLength",
			"������������ �������� ����� ���� systemId");
		__reg_tc__("addCorrectSme.systemIdEmpty",
			"������ ���� systemId");
		//addIncorrectSme
		__reg_tc__("addIncorrectSme",
			"����������� sme � ������������� �����������");
		__reg_tc__("addIncorrectSme.invalidSystemId",
			"����������� 2-�� � ����� sme � ���������� ��������� systemId");
		//deleteSme
		__reg_tc__("deleteSme", "�������� sme");
		__reg_tc__("deleteSme.existentSme", "�������� ������������������� sme");
		__reg_tc__("deleteSme.nonExistentSme",
			"�������� ���������������������/��������������� sme");
		//getSme
		__reg_tc__("getSme", "����� sme �� systemId � �������");
		__reg_tc__("getSme.existentSme", "����� ������������������� sme");
		__reg_tc__("getSme.nonExistentSme", "����� ��������������������� sme");
		//other
		/*
		__reg_tc__("disableExistentSme", "Disable ������������������� sme");
		__reg_tc__("disableNonExistentSme",
			"Disable ���������������������/��������������� sme");
		__reg_tc__("enableExistentSme", "Enable ������������������� sme");
		__reg_tc__("enableNonExistentSme",
			"Enable ���������������������/��������������� sme");
		*/
		__reg_tc__("iterateSme", "������������ �� ������ ������������������ sme");
		__reg_tc__("registerSmeProxy", "����������� sme ������");
	}
};

}
}
}

#endif /* TEST_SMEMAN_SME_MANAGER_CHECK_LIST */

