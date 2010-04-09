#include <cstdio>
#include "TaskTypes.hpp"
#include "CsvStore.hpp"
#include "core/buffers/FixedLengthString.hpp"

int main()
{
    smsc::infosme::CsvStore::CsvFile::Record rec;
    rec.msg.abonent = ".1.1.79137654079";
    printf("sizeof(std::string)=%u\n",sizeof(std::string));
    printf("sizeof(FixedLengthString)=%u\n",sizeof(smsc::core::buffers::FixedLengthString<16>));
    printf("sizeof(CsvStore::Rec)=%u\n",sizeof(rec));
    printf("sizeof(Message)=%u\n",sizeof(rec.msg));
    printf("Please add 5 + %u + len(userData)+1 + len(message)+1\n",rec.msg.abonent.size()+1);
    return 0;
}
