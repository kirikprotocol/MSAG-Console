#include "TestConfig.hpp"

namespace smsc {
namespace test {
namespace conf {

TestConfig TestConfig::cfg = TestConfig();

TestConfig::TestConfig()
{
	//конфигурационный параметр core/reschedule_table
	vector<int> rescheduleTimes;
	rescheduleTimes.push_back(30);
	rescheduleTimes.push_back(20);
	intArrMap["rescheduleTimes"] = rescheduleTimes;
	//различные времена
	intMap["timeCheckAccuracy"] = 5; //5 сек
	intMap["sequentialPduInterval"] = 10; //10 сек
	intMap["maxValidPeriod"] = 150; //150 сек
	intMap["maxWaitTime"] = 60; //max(waitTime)
	intMap["maxDeliveryPeriod"] = 120; //max(validTime - waitTime)
	intMap["minSmeTimeOut"] = intMap["timeCheckAccuracy"];
	intMap["maxSmeTimeOut"] = //не проверяю когда sme timeout > reschedule time
		*min_element(rescheduleTimes.begin(), rescheduleTimes.end()) -
		intMap["timeCheckAccuracy"];
	//smsc sme
	addrMap["smscAddr"] = "+11111111111";
	addrMap["smscAlias"] = ".3.3.smscSmeAlias123";
	strMap["smscSystemId"] = "smscsme";
	strMap["smscServiceType"] = "SmSc";
	intMap["smscProtocolId"] = 3;
	//profiler
	addrMap["profilerAddr"] = "+222222222222";
	addrMap["profilerAlias"] = ".5.5.profilerAlias123";
	strMap["profilerSystemId"] = "profiler";
	strMap["profilerServiceType"] = "pRof";
	intMap["profilerProtocolId"] = 5;
	//db sme
	addrMap["dbSmeAddr"] = "+3333333333333";
	addrMap["dbSmeAlias"] = ".7.7.dbSmeAlias123";
	addrMap["dbSmeInvalidAddr"] = ".7.7.dbSmeInvalidAddr123";
	strMap["dbSmeSystemId"] = "dbSme";
	strMap["dbSmeServiceType"] = "dBsmE";
	intMap["dbSmeProtocolId"] = 23;
	//map proxy
	strMap["mapProxySystemId"] = "MAP_PROXY";
	//abonent info
	addrMap["abonentInfoAddr"] = "+444444444444";
	strMap["abonentInfoSystemId"] = "abonentinfo";
	//респонсы профайлера
	strMap["profilerRespReportNone"] = 
		"Уведомления доставки отключены (Delivery reports are disabled)";
	strMap["profilerRespReportFull"] =
		"Delivery reports are enabled";
	strMap["profilerRespDataCodingDefault"] =
		"Установлен режим приема сообщений в 7-bit кодировке (message character encoding is set to 7-bit)";
	strMap["profilerRespDataCodingUcs2"] =
		"Message character encoding is set to ucs2 encoding (now you will be able to receive messages with russian characters in the case your cell phone supports ucs2 messages)";
	strMap["profilerRespInvalidCmdText"] =
		"Неправильная команда (!@#$%^&*( )_+-=|\\:;'<,>.?/)";
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

