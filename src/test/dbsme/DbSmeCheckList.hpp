#ifndef TEST_DBSME_DBSME_CHECK_LIST
#define TEST_DBSME_DBSME_CHECK_LIST

#include "test/sme/SmppProtocolCheckList.hpp"

namespace smsc {
namespace test {
namespace dbsme {

using smsc::test::sme::SmppProtocolCheckList;

#define __reg_tc__(id, desc) \
	registerTc(id, desc)
	
class DbSmeCheckList : public SmppProtocolCheckList
{
public:
	DbSmeCheckList()
		: SmppProtocolCheckList("Результаты функционального тестирования db sme", "dbsme.chk")
	{
		//submitCorrectDbSmeCmd
		__reg_tc__("submitCorrectDbSmeCmd", "Отправка правильной команды db sme");
		//submitIncorrectDbSmeCmd
		__reg_tc__("submitIncorrectDbSmeCmd", "Отправка неправильной команды db sme");
		//processDbSmeResp
		__reg_tc__("processDbSmeResp", "Ответные сообщения от db sme правильные");
	}
};

}
}
}

#endif /* TEST_DBSME_DBSME_CHECK_LIST */

