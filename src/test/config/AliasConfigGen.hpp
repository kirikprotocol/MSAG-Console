#ifndef TEST_CONFIG_ALIAS_CONFIG_GEN
#define TEST_CONFIG_ALIAS_CONFIG_GEN

#include "test/core/AliasRegistry.hpp"
#include "ConfigGen.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace config {

using smsc::test::core::AliasRegistry;

class AliasConfigGen : public ConfigGen
{
	const AliasRegistry* aliasReg;
public:
	AliasConfigGen(const AliasRegistry* _aliasReg)
		: aliasReg(_aliasReg)
	{
		__require__(aliasReg);
	}
	virtual ~AliasConfigGen() {}
	virtual void saveConfig(const char* configFileName);
};

}
}
}

#endif /* TEST_CONFIG_ALIAS_CONFIG_GEN */

