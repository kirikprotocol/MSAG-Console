#ifndef TEST_ALIAS_ALIAS_MANAGER_CHECK_LIST
#define TEST_ALIAS_ALIAS_MANAGER_CHECK_LIST

#include "test/util/CheckList.hpp"

namespace smsc {
namespace test {
namespace alias {

using smsc::test::util::CheckList;

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
class AliasManagerCheckList : public CheckList
{
public:
	AliasManagerCheckList()
		: CheckList("–езультаты функционального тестировани€ Alias Manager", "alias_man.chk")
	{
		//addCorrectAlias
		__reg_tc__("addCorrectAlias",
			"–егистраци€ алиаса дл€ заданной (случайно сгенеренной) пары (адрес, алиас)");
		__reg_tc__("addCorrectAlias.allMatchNoSubstSymbols",
			"јдрес и алиас в точности соответствуют исходным (без знаков подстановки)");
		__reg_tc__("addCorrectAlias.allMatchWithQuestionMarks",
			"јдрес и алиас соответствуют исходным с одним или несколькими '?' в конце");
		__reg_tc__("addCorrectAlias.allMatchEntirelyQuestionMarks",
			"јдрес и алиас соответствуют исходным, самый короткий или оба целиком из '?'");
		__reg_tc__("addCorrectAlias.allMatchWithAsterisks",
			"јдрес и алиас соответствуют исходным со '*' в конце");
		//addCorrectAliasNotMatchAddress
		__reg_tc__("addCorrectAlias.addrNotMatch",
			"јлиас соответствует исходному, адрес отличаетс€");
		__reg_tc__("addCorrectAlias.addrNotMatch.diffType",
			"ќтличающийс€ typeOfNumber");
		__reg_tc__("addCorrectAlias.addrNotMatch.diffPlan",
			"ќтличающийс€ numberingPlan");
		__reg_tc__("addCorrectAlias.addrNotMatch.diffValue",
			"¬ строке адреса отличаетс€ один символ, но длина строки соответствует");
		__reg_tc__("addCorrectAlias.addrNotMatch.diffValueLength",
			"¬ строке адреса все символы соответствуют исходным, но количество '?' делает строку отличающейс€ длины");
		//addCorrectAliasNotMatchAlias
		__reg_tc__("addCorrectAlias.aliasNotMatch",
			"јдрес соответствует исходному, алиас отличаетс€");
		__reg_tc__("addCorrectAlias.aliasNotMatch.diffType",
			"ќтличающийс€ typeOfNumber");
		__reg_tc__("addCorrectAlias.aliasNotMatch.diffPlan",
			"ќтличающийс€ numberingPlan");
		__reg_tc__("addCorrectAlias.aliasNotMatch.diffValue",
			"¬ строке алиаса отличаетс€ один символ, но длина строки соответствует");
		__reg_tc__("addCorrectAlias.aliasNotMatch.diffValueLength",
			"¬ строке алиаса все символы соответствуют исходным, но количество '?' делает строку отличающейс€ длины");
		//addCorrectAliasException
		__reg_tc__("addCorrectAliasException",
			"–егистраци€ алиаса со '*' в конце с переполнением адреса при alias->addr или алиаса при addr->alias");
		/*
		__reg_tc__("addIncorrectAlias",
			"–егистраци€ алиаса с некорректными параметрами");
		*/
		__reg_tc__("deleteAliases",
			"ќбнуление таблицы алиасов");
		__reg_tc__("findAliasByAddress",
			"ѕравильный результат поиска алиаса по заданному адресу (из списка случайно сгенеренных адресов)");
		__reg_tc__("findAddressByAlias",
			"ѕравильный результат поиска адреса по заданному алиасу (из списка случайно сгенеренных алиасов)");
		__reg_tc__("checkInverseTransformation",
			"ѕреобразование addr->alias->addr возвращает исходный адрес (дл€ alias->addr->alias это необ€зательно)");
		__reg_tc__("iterateAliases",
			"»терирование по списку зарегистрированных алиасов");
	}
};

}
}
}

#endif /* TEST_ALIAS_ALIAS_MANAGER_CHECK_LIST */

