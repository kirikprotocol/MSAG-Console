#ifndef TEST_CONFIG_ALIAS_CONFIG_GEN
#define TEST_CONFIG_ALIAS_CONFIG_GEN

#include "test/core/AliasRegistry.hpp"
#include "ConfigGen.hpp"
#include "test/util/CheckList.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace config {

using smsc::test::core::AliasRegistry;
using smsc::test::util::CheckList;

class AliasConfigGen : public ConfigGen
{
	const AliasRegistry* aliasReg;
	CheckList* chkList;
public:
	AliasConfigGen(const AliasRegistry* _aliasReg, CheckList* _chkList)
		: aliasReg(_aliasReg), chkList(_chkList)
	{
		__require__(aliasReg);
		//__require__(chkList);
	}
	virtual ~AliasConfigGen() {}
	virtual void saveConfig(const char* configFileName);
};

}
}
}

#endif /* TEST_CONFIG_ALIAS_CONFIG_GEN */

