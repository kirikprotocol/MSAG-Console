#include "TestConfig.hpp"

namespace smsc {
namespace test {
namespace conf {

TestConfig TestConfig::cfg = TestConfig();

TestConfig::TestConfig()
{
	//���������������� �������� core/reschedule_table
	vector<int> rescheduleTimes;
	rescheduleTimes.push_back(40);
	rescheduleTimes.push_back(30);
	intArrMap["rescheduleTimes"] = rescheduleTimes;
	//��������� �������
	intMap["timeCheckAccuracy"] = 5; //5 ���
	intMap["missingPduCheckInterval"] = 20; //20 ���
	intMap["scCmdTimeout"] = 8; //���� ����� ����� ������� � SC
	intMap["smeInactivityTime"] = 10; //������ ������������ sme �� ������ enquire_link
	intMap["smeInactivityTimeOut"] = 15; //����������� ���������� ����� ������ enquire_link
	intMap["proxyTimeout"] = 2; //������� ��� ��������� ������
	intMap["maxValidPeriod"] = 150; //150 ���
	intMap["maxWaitTime"] = 60; //max(waitTime)
	intMap["maxDeliveryPeriod"] = 120; //max(validTime - waitTime)
	intMap["minSmeTimeOut"] = intMap["timeCheckAccuracy"];
	intMap["maxSmeTimeOut"] = //�� �������� ����� sme timeout > reschedule time
		*min_element(rescheduleTimes.begin(), rescheduleTimes.end()) -
		intMap["timeCheckAccuracy"];
	//smsc sme
	addrMap["smscAddr"] = "+11111111111"; //11 ��������
	addrMap["smscAlias"] = "111111111110";
	strMap["smscSystemId"] = "smscsme";
	strMap["smscServiceType"] = "SmSc";
	intMap["smscProtocolId"] = 3;
	//profiler
	addrMap["profilerAddr"] = "+222222222222"; //12 ��������
	addrMap["profilerAlias"] = "2222222222220";
	strMap["profilerSystemId"] = "profiler";
	strMap["profilerServiceType"] = "pRof";
	intMap["profilerProtocolId"] = 5;
	//db sme
	addrMap["dbSmeAddr"] = "+3333333333333"; //13 ��������
	addrMap["dbSmeAlias"] = "33333333333330";
	addrMap["dbSmeInvalidAddr"] = ".7.7.dbSmeInvalidAddr123";
	strMap["dbSmeSystemId"] = "dbSme";
	strMap["dbSmeServiceType"] = "dBsmE";
	intMap["dbSmeProtocolId"] = 7;
	strMap["dbSmePassword"] = "dbSmePwd";
	//map proxy
	strMap["mapProxySystemId"] = "MAP_PROXY";
	//abonent info
	addrMap["abonentInfoAddrSme"] = "+44444444444444"; //14 ��������
	addrMap["abonentInfoAliasSme"] = "444444444444440";
	addrMap["abonentInfoAddrMobile"] = "+555555555555555"; //15 ��������
	addrMap["abonentInfoAliasMobile"] = "5555555555555550";
	strMap["abonentInfoSystemId"] = "abonentinfo";
	strMap["abonentInfoServiceType"] = "AboN";
	intMap["abonentInfoProtocolId"] = 10;
	strMap["abonentInfoPassword"] = "abonentInfoPasswd";
	//email sme
	strMap["smtpHost"] = "dvl.novosoft.ru";
	strMap["mailbox"] = "bryz";
	addrMap["emailSmeAddr"] = "+6666666666666666"; //16 ��������
	addrMap["emailSmeAlias"] = "66666666666666660";
	strMap["emailSmeSystemId"] = "email";
	strMap["emailSmeServiceType"] = "EmaiL";
	intMap["emailSmeProtocolId"] = 13;
	strMap["emailSmePassword"] = "emailSmePasswd";
	//distr list sme
	addrMap["distrListAddr"] = "+77777777777777777"; //17 ��������
	addrMap["distrListAlias"] = "777777777777777770";
	strMap["distrListSystemId"] = "DSTRLST";
	strMap["distrListServiceType"] = "DstL";
	intMap["distrListProtocolId"] = 15;
	//�������� db sme
	strMap["dbSmeRespProviderNoFound"] = "����� ���������� �� ���������������";
	strMap["dbSmeRespJobNotFound"] = "Job not found";
	strMap["dbSmeRespDsFailure"] = "������ ���������� ������� (������������� ����, ������ ����� ������, ���������� ������ ������� ���� ������)";
	strMap["dbSmeRespDsConnLost"] = "Connection to data source is lost (this may be caused by internal database server errors, network problems, all the other possible and impossible errors all around the world)";
	strMap["dbSmeRespDsStmtFailed"] = "������ � SQL ������� (!@#$%^&*( )_+-=|\\:;'<,>.?/)";
	strMap["dbSmeRespQueryNull"] = "������� ��������� ������� (Query result is NULL)";
	strMap["dbSmeRespInputParse"] = "������ �� ������� ���������� (Input parsing error)";
	strMap["dbSmeRespOutputFormat"] = "������ �������������� ���������� (Output formatting error)";
	strMap["dbSmeRespInvalidConfig"] = "������ ������������ (Configuration is invalid)";
}

int TestConfig::getIntParam(const char* name)
{
	IntMap::const_iterator it = cfg.intMap.find(name);
	__require__(it != cfg.intMap.end());
	return it->second;
}

const vector<int>& TestConfig::getIntArrParam(const char* name)
{
	IntArrMap::const_iterator it = cfg.intArrMap.find(name);
	__require__(it != cfg.intArrMap.end());
	return it->second;
}

const string& TestConfig::getStrParam(const char* name)
{
	StrMap::const_iterator it = cfg.strMap.find(name);
	__require__(it != cfg.strMap.end());
	return it->second;
}

const Address& TestConfig::getAddrParam(const char* name)
{
	AddrMap::const_iterator it = cfg.addrMap.find(name);
	__require__(it != cfg.addrMap.end());
	return it->second;
}

}
}
}

