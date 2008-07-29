#pragma ident "$Id$"
/* ************************************************************************** *
 * Helper classes for operations with RPCauses (Reject Processing Cause)
 * ************************************************************************** */
#ifndef __SMSC_INMAN_RPCLIST__
#define __SMSC_INMAN_RPCLIST__

#include <list>
#include <stdlib.h>

#include "util/Exception.hpp"
using smsc::util::format;
using smsc::util::CustomException;

#include "inman/common/CSVList.hpp"
using smsc::util::CSVList;

namespace smsc   {
namespace inman  {
namespace common {

//Ordinary list of RPCauses
class RPCList : public std::list<unsigned char> {
public:
    //format: "C1, C2, ..., Cn"
    size_type init(const char * str) throw(CustomException)
    {
        if (!str || !str[0])
            RPCList::clear();
        else {
            std::string rplist(str);
            std::string::size_type pos = 0, commaPos;
            do {
                commaPos = rplist.find_first_of(',', pos);
                std::string rp_s(rplist.substr(pos,
                        ((commaPos != rplist.npos) ? commaPos : rplist.size()) - pos));
                int rp_i = atoi(rp_s.c_str());
                if (!rp_i || (rp_i > 0xFF))
                    throw CustomException("bad element \'%s\'", rp_s.c_str());
                RPCList::push_back((unsigned char)rp_i);
                pos = commaPos + 1;
            } while (commaPos != rplist.npos);
        }
        return RPCList::size();
    }
    size_type print(std::string & ostr)
    {
        size_type i = 0;
        for (RPCList::iterator it = RPCList::begin(); it != RPCList::end(); ++it, ++i)
            format(ostr, "%s%u", i ? ", ":"", (*it));
        return i;
    }

    bool exist(unsigned char rpc) const
    {
        for (RPCList::const_iterator it = RPCList::begin(); it != RPCList::end(); ++it) {
            if (*it == rpc)
                return true;
        }
        return false;
    }
};

//RPCause with unsigned int attribute
typedef std::pair<unsigned char, unsigned> RPCauseATT;

class RPCListATT : public std::list<RPCauseATT> {
public:
    //format: "C1:A1, C2:A2, ..., Cn:An"
    size_type init(const char * str) throw(CustomException)
    {
        if (!str || !str[0])
            RPCListATT::clear();
        else {
            std::string rplist(str);
            std::string::size_type pos = 0, commaPos;
            do {
                commaPos = rplist.find_first_of(',', pos);
                std::string rp_s(rplist.substr(pos,
                        ((commaPos != rplist.npos) ? commaPos : rplist.size()) - pos));
                //separate RPCause ant attribute
                CSVList  rpAtt(':');
                if (!rpAtt.init(rp_s.c_str()))
                    throw CustomException("bad element \'%s\'", rp_s.c_str());
                //convert RPCause
                int rp_i = atoi(rpAtt[0].c_str());
                if ((rp_i <= 0) || (rp_i > 0xFF))
                    throw CustomException("bad element \'%s\'", rp_s.c_str());
                //convert attribute
                int att = 0;
                if (rpAtt.size() > 1) {
                    if ((att = atoi(rpAtt[1].c_str())) <= 0)
                        throw CustomException("bad element \'%s\'", rp_s.c_str());
                }
                RPCListATT::push_back(RPCauseATT(rp_i, att));
                pos = commaPos + 1;
            } while (commaPos != rplist.npos);
        }
        return RPCListATT::size();
    }
    size_type print(std::string & ostr)
    {
        size_type i = 0;
        for (RPCListATT::iterator it = RPCListATT::begin(); it != RPCListATT::end(); ++it, ++i)
            format(ostr, "%s%u:%u", i ? ", ":"", it->first, it->second);
        return i;
    }

    const RPCauseATT * exist(unsigned char rpc) const
    {
        for (RPCListATT::const_iterator it = RPCListATT::begin(); it != RPCListATT::end(); ++it) {
            if (it->first == rpc)
                return it.operator->();
        }
        return NULL;
    }
};


} //common
} //inman
} //smsc

#endif /* __SMSC_INMAN_RPCLIST__ */

