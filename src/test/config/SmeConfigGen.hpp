#ifndef TEST_CONFIG_SME_CONFIG_GEN
#define TEST_CONFIG_SME_CONFIG_GEN

#include "test/core/SmeRegistry.hpp"
#include "ConfigGen.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace config {

using smsc::test::core::SmeRegistry;

class SmeConfigGen : public ConfigGen
{
	const SmeRegistry* smeReg;

public:
	SmeConfigGen(const SmeRegistry* _smeReg)
		: smeReg(_smeReg)
	{
		__require__(smeReg);
	}
	virtual ~SmeConfigGen() {}
	virtual void saveConfig(const char* configFileName);
};

}
}
}

#endif /* TEST_CONFIG_SME_CONFIG_GEN */

