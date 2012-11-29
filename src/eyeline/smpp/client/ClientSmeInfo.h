#ifndef _EYELINE_SMPP_CLIENTSMEINFO_H
#define _EYELINE_SMPP_CLIENTSMEINFO_H

#include "eyeline/smpp/SmeInfo.h"

namespace eyeline {
namespace smpp {

struct ClientSmeInfo : public SmeInfo
{
    std::string host;
    int         port;
};

}
}

#endif
