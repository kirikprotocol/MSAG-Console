#include "SCAGCommand.h"

namespace scag { namespace transport {

    Hash<TransportType> SCAGCommand::InitTransportTypeHash()
    {
        Hash<TransportType> res;
        res["smpp"] = SMPP;
        res["http"] = HTTP;
        res["mms"] = MMS;
        return res;
    }

    Hash<TransportType> SCAGCommand::TransportTypeHash = SCAGCommand::InitTransportTypeHash();

}}

