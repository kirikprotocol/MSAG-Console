/* ************************************************************************** *
 * Helpers: Character Separated Values List.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_CSVLIST__
#ident "@(#)$Id$"
#define __SMSC_UTIL_CSVLIST__

#include <vector>
#include "util/vformat.hpp"
#include "inman/common/strutil.hpp"

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

        std::string csv_list(str);
        if (_cutBS && str_cut_blanks(csv_list).empty())
            return 0;

        std::string::size_type pos = 0, dlmPos;
        do {
            dlmPos = csv_list.find_first_of(_dlm, pos);
            std::string rp_s(csv_list.substr(pos, 
                ((dlmPos != csv_list.npos) ? dlmPos : csv_list.size()) - pos));
            if (_cutBS)
                str_cut_blanks(rp_s);
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

