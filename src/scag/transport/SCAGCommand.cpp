#include "SCAGCommand.h"

namespace scag { namespace transport {

    Hash<TransportType> SCAGCommand::InitTransportTypeHash()
    {
        Hash<TransportType> res;
        res["SMPP"] = SMPP;
        res["HTTP"] = HTTP;
        res["MMS"] = MMS;
        res["WAP"] = WAP;
        return res;
    }

    Hash<TransportType> SCAGCommand::TransportTypeHash = SCAGCommand::InitTransportTypeHash();

}}

