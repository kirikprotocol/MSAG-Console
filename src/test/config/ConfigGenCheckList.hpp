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
		: CheckList("Результаты функционального тестирования конфигурации SC", "config.chk")
	{
		//aliasConfig
		__reg_tc__("aliasConfig",
			"Конфигурация alias manager (aliases.xml)");
		__reg_tc__("aliasConfig.correctAlias",
			"Корректный алиас (полное соответствие требованиям и ограничениям согласно спецификации)");
		__reg_tc__("aliasConfig.incorrectAlias",
			"Некорректный алиас ...........");
		//smeConfig
		__reg_tc__("smeConfig",
			"Конфигурация sme manager (sme.xml)");
		__reg_tc__("smeConfig.correctSme",
			"Корректный sme");
		__reg_tc__("smeConfig.incorrectSme",
			"Некорректный sme ...........");
		//routeConfig
		__reg_tc__("routeConfig",
			"Конфигурация route manager (routes.xml)");
		__reg_tc__("routeConfig.billing",
			"Маршруты с включенным биллингом");
		__reg_tc__("routeConfig.archivate",
			"Маршруты с включенной архивацией sms");
		__reg_tc__("routeConfig.notEnabling",
			"Запрещающие маршруты");
		//routeConfig.routeSource
		__reg_tc__("routeConfig.routeSource",
			"Задание отправителя (тег <source>) в маршруте");
		__reg_tc__("routeConfig.routeSource.subject",
			"Использование субъектов для задания отправителя");
		__reg_tc__("routeConfig.routeSource.subject.fakeMaskBefore",
			"В субъекте перед тестируемой маской адреса отправителя есть другие маски (тег <mask>)");
		__reg_tc__("routeConfig.routeSource.subject.fakeMaskAfter",
			"В субъекте после тестируемой маски адреса отправителя есть другие маски (тег <mask>)");
		__reg_tc__("routeConfig.routeSource.mask",
			"Использование маски адреса для задания отправителя");
		__reg_tc__("routeConfig.routeSource.fakeSourceBefore",
			"В маршруте перед тестируемым отправителем (тег <source>) есть другие отправители");
		__reg_tc__("routeConfig.routeSource.fakeSourceAfter",
			"В маршруте после тестируемого отправителя (тег <source>) есть другие отправители");
		//routeConfig.routeDest
		__reg_tc__("routeConfig.routeDest",
			"Задания получателя (тег <dest>) в маршруте");
		__reg_tc__("routeConfig.routeDest.subject",
			"Использование субъектов для задания получателя");
		__reg_tc__("routeConfig.routeDest.subject.fakeMaskBefore",
			"В субъекте перед тестируемой маской адреса получателя есть другие маски (тег <mask>)");
		__reg_tc__("routeConfig.routeDest.subject.fakeMaskAfter",
			"В субъекте после тестируемой маски адреса получателя есть другие маски (тег <mask>)");
		__reg_tc__("routeConfig.routeDest.mask",
			"Использование маски адреса для задания получателя");
		__reg_tc__("routeConfig.routeDest.fakeDestBefore",
			"В маршруте перед тестируемым получателем (тег <dest>) есть другие получатели");
		__reg_tc__("routeConfig.routeDest.fakeDestAfter",
			"В маршруте после тестируемого получателя (тег <dest>) есть другие получатели");
		//routeConfig.incorrectRoute
		__reg_tc__("routeConfig.incorrectRoute",
			"Некорректный маршрут ...........");
	}
};

}
}
}

#endif /* TEST_CONFIG_CONFIG_GEN_CHECK_LIST */
