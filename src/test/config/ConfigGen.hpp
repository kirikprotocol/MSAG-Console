#ifndef TEST_CONFIG_CONFIG_GEN
#define TEST_CONFIG_CONFIG_GEN

namespace smsc {
namespace test {
namespace config {

struct ConfigGen
{
	virtual void saveConfig(const char* configFileName) = NULL;
};

}
}
}

#endif /* TEST_CONFIG_CONFIG_GEN */

