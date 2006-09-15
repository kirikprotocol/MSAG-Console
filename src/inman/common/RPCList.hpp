#ident "$Id$"
#ifndef __SMSC_INMAN_RPCLIST__
#define __SMSC_INMAN_RPCLIST__

#include <list>
#include <stdlib.h>

//#include "inman/common/util.hpp"
#include "inman/common/errors.hpp"

namespace smsc   {
namespace inman  {
namespace common {

class RPCList : public std::list<unsigned char> {
public:
    int init(const char * str) throw(CustomException)
    {
        if (!str || !str[0])
            RPCList::clear();
        else {
            std::string rplist(str);
            std::string::size_type pos = 0, commaPos;
            do {
                commaPos = rplist.find_first_of(',', pos);
                std::string rp_s = rplist.substr(pos, commaPos);
                int rp_i = atoi(rp_s.c_str());
                if (!rp_i || (rp_i > 0xFF))
                    throw CustomException(format("bad element \'%s\'", rp_s.c_str()).c_str());

                RPCList::push_back((unsigned char)rp_i);
                pos = commaPos + 1;
            } while (commaPos != rplist.npos);
        }
        return RPCList::size();
    }
    int print(std::string & ostr)
    {
        int i = 0;
        RPCList::iterator it = RPCList::begin();
        for (; it != RPCList::end(); it++, i++)
            format(ostr, "%s%u", i ? ", ":"", (*it));
        return i;
    }
};

} //common
} //inman
} //smsc

#endif /* __SMSC_INMAN_RPCLIST__ */

