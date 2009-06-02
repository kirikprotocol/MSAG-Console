/* ************************************************************************** *
 * Helpers: Character Separated Values List.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_CSVLIST__
#ident "@(#)$Id$"
#define __SMSC_UTIL_CSVLIST__

#include <vector>
#include "util/vformat.hpp"

namespace smsc {
namespace util {

class STDString : public std::string {
public:
    STDString() : std::string()
    { }
    STDString(const char * use_cstr) : std::string(use_cstr)
    { }
    STDString(const std::string & use_str) : std::string(use_str)
    { }

    //Cuts off leading/ending blanks
    static std::string & cutBlanks(std::string & use_str,
                               const char * pattern = " \t\r\n")
    {
        if (!use_str.empty()) {
            //erase leading blanks
            std::string::size_type 
                bpos = use_str.find_first_not_of(pattern);
            if (bpos == use_str.npos) { //string contains only blank chars
              use_str.clear();
            } else {
              if (bpos)
                  use_str.erase(0, bpos);
  
              //erase ending blanks
              bpos = use_str.find_last_not_of(pattern);
              if (bpos != use_str.npos)
                  use_str.erase(bpos + 1, use_str.npos);
            }
        }
        return use_str;
    }

    STDString & cutBlanks(const char * pattern = " \t\r\n")
    {
        return (STDString &)cutBlanks(*this, pattern);
    }
};

//Character Separated Values List (delimiter is comma by default)
class CSVList : public std::vector<std::string> {
private:
    char _dlm;
    bool _cutBS;

public:
    CSVList(char use_dlm = ',', bool cut_blanks = true)
        : std::vector<std::string>(), _dlm(use_dlm), _cutBS(cut_blanks)
    { }

    CSVList(const char * str, char use_dlm = ',', bool cut_blanks = true)
        : std::vector<std::string>(), _dlm(use_dlm), _cutBS(cut_blanks)
    { 
        init(str);
    }

    //returns number of strings separated
    CSVList::size_type init(const char * str)
    {
        clear();
        if (!str || !str[0])
             return 0;

        STDString csv_list(str);
        if (_cutBS && csv_list.cutBlanks().empty())
            return 0;

        std::string::size_type pos = 0, dlmPos;
        do {
            dlmPos = csv_list.find_first_of(_dlm, pos);
            STDString rp_s(csv_list.substr(pos, 
                ((dlmPos != csv_list.npos) ? dlmPos : csv_list.size()) - pos));
            if (_cutBS)
                rp_s.cutBlanks();
            push_back(rp_s);
            pos = dlmPos + 1;
        } while (dlmPos != csv_list.npos);
        return size();
    }

    int print(std::string & ostr, bool ins_space = true) const
    {
        int i = 0;
        char dlmStr[3];

        dlmStr[0] = _dlm;
        dlmStr[1] = ins_space ? ' ' : 0;
        dlmStr[2] = 0;
        CSVList::const_iterator it = CSVList::begin();
        for (; it != CSVList::end(); ++it, ++i)
            format(ostr, "%s%s", i ? dlmStr : "", it->c_str());
        return i;
    }

    std::string print(bool ins_space = true) const
    {
        std::string elems;
        print(elems, ins_space);
        return elems;
    }
};

} //util
} //smsc

#endif /* __SMSC_UTIL_CSVLIST__ */

