#ifndef TEST_TEST_CONFIG
#define TEST_TEST_CONFIG

#include "sms/sms.h"

namespace smsc {
namespace test {

using smsc::sms::Address;

//конфигурационный параметр core/reschedule_table
const int rescheduleTimes[] = {30, 20};
const int rescheduleSize = sizeof(rescheduleTimes) / sizeof(*rescheduleTimes);

const int timeCheckAccuracy = 5; //5 сек
const int sequentialPduInterval = 10; //10 сек
const int maxValidPeriod = 86400; //1 сутки
const int maxWaitTime = 60; //max(waitTime)
const int maxDeliveryPeriod = 120; //max(validTime - waitTime)

//адрес SC
const uint8_t smscAddrTon = 3;
const uint8_t smscAddrNpi = 5;
static const char* smscAddrVal = "123abc";
static const Address smscAddr(strlen(smscAddrVal), smscAddrTon, smscAddrNpi, smscAddrVal);

}
}

#endif /* TEST_TEST_CONFIG */

