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
		: CheckList("Результаты функционального тестирования Sme Manager", "sme_man.chk")
	{
		//addCorrectSme
		__reg_tc__("addCorrectSme", "Регистрация sme с корректными параметрами");
		__reg_tc__("addCorrectSme.invalidType",
			"Значение typeOfNumber вне диапазона разрешенного SMPP v3.4");
		__reg_tc__("addCorrectSme.invalidPlan",
			"Значение numberingPlan вне диапазона разрешенного SMPP v3.4");
		__reg_tc__("addCorrectSme.invalidVersion",
			"Значение interfaceVersion вне диапазона разрешенного SMPP v3.4");
		__reg_tc__("addCorrectSme.invalidAddressRangeLength",
			"Недопустимое значение длины поля addressRange");
		__reg_tc__("addCorrectSme.invalidSystemTypeLength",
			"Недопустимое значение длины поля systemType");
		__reg_tc__("addCorrectSme.invalidPasswordLength",
			"Недопустимое значение длины поля password");
		__reg_tc__("addCorrectSme.invalidSystemIdLength",
			"Недопустимое значение длины поля systemId");
		__reg_tc__("addCorrectSme.systemIdEmpty",
			"Пустое поле systemId");
		//addIncorrectSme
		__reg_tc__("addIncorrectSme",
			"Регистрация sme с некорректными параметрами");
		__reg_tc__("addIncorrectSme.invalidSystemId",
			"Регистрация 2-ух и более sme с одинаковым значением systemId");
		//deleteSme
		__reg_tc__("deleteSme", "Удаление sme");
		__reg_tc__("deleteSme.existentSme", "Удаление зарегистрированного sme");
		__reg_tc__("deleteSme.nonExistentSme",
			"Удаление незарегистрированного/несуществующего sme");
		//getSme
		__reg_tc__("getSme", "Поиск sme по systemId и индексу");
		__reg_tc__("getSme.existentSme", "Поиск зарегистрированного sme");
		__reg_tc__("getSme.nonExistentSme", "Поиск незарегистрированного sme");
		//other
		/*
		__reg_tc__("disableExistentSme", "Disable зарегистрированного sme");
		__reg_tc__("disableNonExistentSme",
			"Disable незарегистрированного/несуществующего sme");
		__reg_tc__("enableExistentSme", "Enable зарегистрированного sme");
		__reg_tc__("enableNonExistentSme",
			"Enable незарегистрированного/несуществующего sme");
		*/
		__reg_tc__("iterateSme", "Итерирование по списку зарегистрированных sme");
		__reg_tc__("registerSmeProxy", "Регистрация sme прокси");
	}
};

}
}
}

#endif /* TEST_SMEMAN_SME_MANAGER_CHECK_LIST */

