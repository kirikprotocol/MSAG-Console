#ident "$Id$"
#ifndef __SMSC_UTIL_CSVLIST__
#define __SMSC_UTIL_CSVLIST__

#include <vector>
#include <stdlib.h>

namespace smsc {
namespace util {

//Character Separated Values List (delimiter is comma by default)
class CSVList : public std::vector<std::string> {
private:
    char _dlm;
    bool _cutBS;

public:
    CSVList(char use_dlm = ',', bool cut_blanks = true)
        : std::vector<std::string>(), _dlm(use_dlm), _cutBS(cut_blanks)
    { }

    //returns number of strings separated
    CSVList::size_type init(const char * str)
    {
        clear();
        if (!str || !str[0])
             return 0;

        std::string csv_list(str);
        if (_cutBS)
            CSVList::cutBlanks(csv_list);

        std::string::size_type pos = 0, dlmPos;
        do {
            dlmPos = csv_list.find_first_of(_dlm, pos);
            std::string rp_s(csv_list.substr(pos, 
                ((dlmPos != csv_list.npos) ? dlmPos : csv_list.size()) - pos));
            if (_cutBS)
                CSVList::cutBlanks(rp_s);
            push_back(rp_s);
            pos = dlmPos + 1;
        } while (dlmPos != csv_list.npos);
        return size();
    }

    int print(std::string & ostr)
    {
        int i = 0;
        char dlmStr[3];

        dlmStr[0] = _dlm; dlmStr[1] = ' '; dlmStr[2] = 0;
        CSVList::iterator it = CSVList::begin();
        for (; it != CSVList::end(); it++, i++)
            format(ostr, "%s%u", i ? dlmStr : "", it->c_str());
        return i;
    }

    //Cuts off leading/ending blanks
    static std::string & cutBlanks(std::string & use_str)
    {
        if (use_str.empty())
            return use_str;
        std::string::size_type bpos = 0, cnt = 0;
        //erase leading blanks
        while ((use_str[bpos] == ' ') || (use_str[bpos] == '\t'))
            bpos++;
        if (bpos)
            use_str.erase(0, bpos);
        //erase ending blanks
        bpos = use_str.length();
        while (bpos) { 
            bpos--;
            if ((use_str[bpos] == ' ') || (use_str[bpos] == '\t')) {
                cnt++;
            } else {
                if (cnt) 
                    use_str.erase(bpos + 1, cnt);
                break;
            }
        }
        return use_str;
    }
};

} //util
} //smsc

#endif /* __SMSC_UTIL_CSVLIST__ */

