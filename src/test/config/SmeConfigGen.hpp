#ifndef TEST_CONFIG_SME_CONFIG_GEN
#define TEST_CONFIG_SME_CONFIG_GEN

#include "test/core/SmeRegistry.hpp"
#include "ConfigGen.hpp"
#include "test/util/CheckList.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace config {

using smsc::test::core::SmeRegistry;
using smsc::test::util::CheckList;

class SmeConfigGen : public ConfigGen
{
	const SmeRegistry* smeReg;
	CheckList* chkList;
public:
	SmeConfigGen(const SmeRegistry* _smeReg, CheckList* _chkList)
		: smeReg(_smeReg), chkList(_chkList)
	{
		__require__(smeReg);
		//__require__(chkList);
	}
	virtual ~SmeConfigGen() {}
	virtual void saveConfig(const char* configFileName);
};

}
}
}

#endif /* TEST_CONFIG_SME_CONFIG_GEN */

