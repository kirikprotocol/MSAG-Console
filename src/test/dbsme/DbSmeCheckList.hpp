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
		: SmppProtocolCheckList("���������� ��������������� ������������ db sme", "dbsme.chk")
	{
		//submitCorrectDbSmeCmd
		__reg_tc__("submitCorrectDbSmeCmd", "�������� ���������� ������� db sme");
		//submitIncorrectDbSmeCmd
		__reg_tc__("submitIncorrectDbSmeCmd", "�������� ������������ ������� db sme");
		//processDbSmeResp
		__reg_tc__("processDbSmeResp", "�������� ��������� �� db sme ����������");
	}
};

}
}
}

#endif /* TEST_DBSME_DBSME_CHECK_LIST */

