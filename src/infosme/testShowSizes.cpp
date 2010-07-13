#include <cstdio>
#include "TaskTypes.hpp"
#include "CsvStore.hpp"
#include "core/buffers/FixedLengthString.hpp"


using smsc::core::buffers::FixedLengthString;

struct TestMsg
{
    uint64_t    id;
    time_t      date;
    FixedLengthString<16> abonent;
    char*       userData;
    unsigned    regionId;
    unsigned    message;
};

struct TestRec {
    TestMsg msg;
    uint8_t state;
};

int main()
{
    smsc::infosme::CsvStore::CsvFile::Record rec;
    rec.msg.abonent = ".1.1.79137654079";
    printf("sizeof(std::string)=%u\n",unsigned(sizeof(std::string)));
    printf("sizeof(FixedLengthString)=%u\n",unsigned(sizeof(smsc::core::buffers::FixedLengthString<16>)));
    printf("sizeof(CsvStore::Rec)=%u\n",unsigned(sizeof(rec)));
    printf("sizeof(Message)=%u\n",unsigned(sizeof(rec.msg)));
    printf("Please add 5 + %u + len(userData)+1 + len(message)+1\n",unsigned(rec.msg.abonent.size()+1));
    printf("\n");
    printf("sizeof(TestMsg)=%u\n",unsigned(sizeof(TestMsg)));
    printf("sizeof(TestRec)=%u\n",unsigned(sizeof(TestRec)));
    printf("Please add len(userData)+1\n");
    return 0;
}
