#ifndef TEST_TEST_CONFIG
#define TEST_TEST_CONFIG

#include "sms/sms.h"

namespace smsc {
namespace test {

using smsc::sms::Address;

//конфигурационный параметр core/reschedule_table
const int rescheduleTimes[] = {20, 10};
const int rescheduleSize = sizeof(rescheduleTimes) / sizeof(int);

const int timeCheckAccuracy = 5; //5 сек
const int maxValidPeriod = 24 * 3600; //1 сутки

//адрес SC
const uint8_t smscAddrTon = 3;
const uint8_t smscAddrNpi = 5;
static const char* smscAddrVal = "123abc";
static const Address smscAddr(strlen(smscAddrVal), smscAddrTon, smscAddrNpi, smscAddrVal);

}
}

#endif /* TEST_TEST_CONFIG */

