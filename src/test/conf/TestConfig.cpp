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
	//адреса
	addrMap["smscAddr"] = ".3.5.123abc";
	//profiler
	addrMap["profilerAddr"] = "+38435435";
	addrMap["profilerAlias"] = ".3.3.profilerAlias123";
	strMap["profilerSystemId"] = "profiler";
	strMap["profilerServiceType"] = "pRof";
	intMap["profilerProtocolId"] = 5;
	//db sme
	addrMap["dbSmeAddr"] = "+1597534682";
	addrMap["dbSmeAlias"] = ".5.5.dbSmeAlias123";
	strMap["dbSmeSystemId"] = "dbSme";
	strMap["dbSmeServiceType"] = "dBsmE";
	intMap["dbSmeProtocolId"] = 23;
	//map proxy
	strMap["mapProxySystemId"] = "MAP_PROXY";
	//респонсы профайлера
	strMap["profilerRespReportNone"] = 
		"Уведомления доставки отключены (Delivery reports disabled)";
	strMap["profilerRespReportFull"] =
		"Уведомления доставки включены (Delivery reports enabled)";
	strMap["profilerRespDataCodingDefault"] =
		"Установлена 7-bit кодировка сообщений (7-bit encoding set)";
	strMap["profilerRespDataCodingUcs2"] =
		"Установлена ucs2 кодировка сообщений (ucs2 encoding set)";
	strMap["profilerRespInvalidCmdText"] =
		"Неправильная команда (Invalid command)";
	//респонсы db sme
	strMap["dbSmeRespProviderNoFound"] = "Адрес не найден (DataProvider not found)";
	strMap["dbSmeRespJobNotFound"] = "Задача не найдена (Job not found)";
	strMap["dbSmeRespDsFailure"] = "Ошибка выполнения запроса (Problem with DataSource)";
	strMap["dbSmeRespDsConnLost"] = "Ошибка соединения (Connection to DataSource lost)";
	strMap["dbSmeRespDsStmtFailed"] = "Ошибка SQL (Failed to create SQL statement)";
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

