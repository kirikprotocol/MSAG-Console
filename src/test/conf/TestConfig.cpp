#include "TestConfig.hpp"

namespace smsc {
namespace test {
namespace conf {

TestConfig TestConfig::cfg = TestConfig();

TestConfig::TestConfig()
{
	//конфигурационный параметр core/reschedule_table
	vector<int> rescheduleTimes;
	rescheduleTimes.push_back(40);
	rescheduleTimes.push_back(30);
	intArrMap["rescheduleTimes"] = rescheduleTimes;
	//различные времена
	intMap["timeCheckAccuracy"] = 5; //5 сек
	intMap["missingPduCheckInterval"] = 20; //20 сек
	intMap["scCmdTimeout"] = 8; //макс время жизни команды в SC
	intMap["smeInactivityTime"] = 10; //период неактивности sme до начала enquire_link
	intMap["smeInactivityTimeOut"] = 15; //закрывается соединение после начала enquire_link
	intMap["proxyTimeout"] = 2; //таймаут для помирания прокси
	intMap["maxValidPeriod"] = 150; //150 сек
	intMap["maxWaitTime"] = 60; //max(waitTime)
	intMap["maxDeliveryPeriod"] = 120; //max(validTime - waitTime)
	intMap["minSmeTimeOut"] = intMap["timeCheckAccuracy"];
	intMap["maxSmeTimeOut"] = //не проверяю когда sme timeout > reschedule time
		*min_element(rescheduleTimes.begin(), rescheduleTimes.end()) -
		intMap["timeCheckAccuracy"];
	//smsc sme
	addrMap["smscAddr"] = "+11111111111"; //11 символов
	addrMap["smscAlias"] = "111111111110";
	strMap["smscSystemId"] = "smscsme";
	strMap["smscServiceType"] = "SmSc";
	intMap["smscProtocolId"] = 3;
	//profiler
	addrMap["profilerAddr"] = "+222222222222"; //12 символов
	addrMap["profilerAlias"] = "2222222222220";
	strMap["profilerSystemId"] = "profiler";
	strMap["profilerServiceType"] = "pRof";
	intMap["profilerProtocolId"] = 5;
	//db sme
	addrMap["dbSmeAddr"] = "+3333333333333"; //13 символов
	addrMap["dbSmeAlias"] = "33333333333330";
	addrMap["dbSmeInvalidAddr"] = ".7.7.dbSmeInvalidAddr123";
	strMap["dbSmeSystemId"] = "dbSme";
	strMap["dbSmeServiceType"] = "dBsmE";
	intMap["dbSmeProtocolId"] = 7;
	strMap["dbSmePassword"] = "dbSmePwd";
	//map proxy
	strMap["mapProxySystemId"] = "MAP_PROXY";
	//abonent info
	addrMap["abonentInfoAddrSme"] = "+44444444444444"; //14 символов
	addrMap["abonentInfoAliasSme"] = "444444444444440";
	addrMap["abonentInfoAddrMobile"] = "+555555555555555"; //15 символов
	addrMap["abonentInfoAliasMobile"] = "5555555555555550";
	strMap["abonentInfoSystemId"] = "abonentinfo";
	strMap["abonentInfoServiceType"] = "AboN";
	intMap["abonentInfoProtocolId"] = 10;
	strMap["abonentInfoPassword"] = "abonentInfoPasswd";
	//email sme
	strMap["smtpHost"] = "dvl.novosoft.ru";
	strMap["mailbox"] = "bryz";
	addrMap["emailSmeAddr"] = "+6666666666666666"; //16 символов
	addrMap["emailSmeAlias"] = "66666666666666660";
	strMap["emailSmeSystemId"] = "email";
	strMap["emailSmeServiceType"] = "EmaiL";
	intMap["emailSmeProtocolId"] = 13;
	strMap["emailSmePassword"] = "emailSmePasswd";
	//distr list sme
	addrMap["distrListAddr"] = "+77777777777777777"; //17 символов
	addrMap["distrListAlias"] = "777777777777777770";
	strMap["distrListSystemId"] = "DSTRLST";
	strMap["distrListServiceType"] = "DstL";
	intMap["distrListProtocolId"] = 15;
	//респонсы db sme
	strMap["dbSmeRespProviderNoFound"] = "Адрес получателя не зарегистрирован";
	strMap["dbSmeRespJobNotFound"] = "Job not found";
	strMap["dbSmeRespDsFailure"] = "Ошибка выполнения запроса (дублирующийся ключ, пустой набор данных, внутренние ошибки сервера базы данных)";
	strMap["dbSmeRespDsConnLost"] = "Connection to data source is lost (this may be caused by internal database server errors, network problems, all the other possible and impossible errors all around the world)";
	strMap["dbSmeRespDsStmtFailed"] = "Ошибка в SQL запросе (!@#$%^&*( )_+-=|\\:;'<,>.?/)";
	strMap["dbSmeRespQueryNull"] = "Нулевой результат запроса (Query result is NULL)";
	strMap["dbSmeRespInputParse"] = "Ошибка во входных параметрах (Input parsing error)";
	strMap["dbSmeRespOutputFormat"] = "Ошибка форматирования результата (Output formatting error)";
	strMap["dbSmeRespInvalidConfig"] = "Ошибка конфигурации (Configuration is invalid)";
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

