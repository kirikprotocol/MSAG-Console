#include <iostream>
#include "CsvRecord.h"

using namespace smsc::util::csv;

int main()
{
    // select one of the modes
    // const char* escapeChars = 0;
    const char* escapeChars = "\n\r\0";

    std::string field[12];
    field[00] = std::string("t b");
    field[01] = std::string("100");
    field[02] = std::string("");
    field[03] = std::string("200");
    field[04] = std::string("");
    field[05] = std::string(",");
    field[06] = std::string("hello, world");
    field[07] = std::string("\"hello, world\"");
    field[8] = std::string("\"");
    field[9] = std::string("\\");
    field[10] = std::string("hi\nthere");
    field[11].append("hi");
    field[11].push_back('\0');
    field[11].append("there");

    std::string line;
    {
        // creation
        CsvRecord cr( escapeChars );
        size_t i = 0;
        for ( ; i < 4; ++i ) {
            cr.addSimpleField(line,field[i].c_str(),field[i].size());
        }
        for ( ; i < 12; ++i ) {
            cr.addField(line,field[i].c_str(),field[i].size());
        }
    }
    std::cout << line << "\r\n";
    std::string readf[12];
    {
        // parsing
        CsvRecord cr( line.c_str(), line.size(), escapeChars ? true : false );
        for ( size_t i = 0 ; i < 12; ++i ) {
            bool simple = cr.isNextFieldSimple();
            if ( simple ) {
                cr.parseSimpleField(readf[i]);
            } else {
                cr.parseField(readf[i]);
            }
            std::cout << ( simple ? "simple " : "" ) << "field parsed: <" << readf[i] << ">, orig was:<" << field[i] << ">" << std::endl;
            if ( readf[i] != field[i] ) {
                std::cerr << "not matched" << std::endl;
            }
        }
    }
}
