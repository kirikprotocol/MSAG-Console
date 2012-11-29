#ifndef _EYELINE_SMPP_TESTPDU2_H
#define _EYELINE_SMPP_TESTPDU2_H

#include "testPdu.h"
//#include "DeliverSmPdu.h"
//#include "SubmitSmPdu.h"
#include "GenericNackPdu.h"
//#include "TlvDefines.h"

namespace eyeline {
namespace smpp {

class GenericNackPduContainer : public PduContainer
{
public:
    GenericNackPduContainer(const uint32_t int_value, const char* str_value, smsc::logger::Logger* logptr)
        : PduContainer(int_value, str_value, logptr)
    {
        dp = new GenericNackPdu(int_value, int_value);
        pdu = dp;
    }
    GenericNackPduContainer(GenericNackPdu* newpdu, smsc::logger::Logger* logptr)
        : PduContainer(1, "", logptr)
    {
        dp = newpdu;
        pdu = dp;
    }
    ~GenericNackPduContainer() { pdu = 0; delete dp; }

protected:
    GenericNackPdu* dp;
};


int test2(const uint32_t int_value, const char* str_value, smsc::logger::Logger* log) {
    GenericNackPduContainer d1(int_value, str_value, log);

    smsc_log_info(log, "generate data with default values int: 0x%x(%d) 0x%x(%d) 0x%x(%d) and str: %s",
            int_value & 0xff, int_value & 0xff,
            int_value & 0xffff, int_value & 0xffff,
            int_value, int_value, str_value);
    if ( !d1.fillPdu(true) ) {
        smsc_log_error(log, "generate data error.");
        return -1;
    }

    smsc_log_info(log, "encode data...");
    PduBuffer pb = d1.getPdu()->encode();

    smsc_log_info(log, "decode data...");
    Pdu* pdu;
    {
        const PduInputBuf inputBuf(pb.getSize(),pb.getBuf());
        pdu = inputBuf.decode();
    }

    smsc_log_info(log, "analyse data...");
    uint32_t cmdid = pdu->getPduTypeId();
    if ( cmdid != d1.getPdu()->getPduTypeId() ) {
        smsc_log_error(log, "Analyse: wrong command_id=%x (%x)", cmdid, d1.getPdu()->getPduTypeId());
        return -2;
    }

    GenericNackPduContainer d2(reinterpret_cast<GenericNackPdu*>(pdu), log);
    if ( !d2.logPdu(true) ) {
        smsc_log_error(log, "log data error.");
        return -3;
    }

    int err = d2.comparePdu(d1.getPdu());
    if ( 0 == err )
        smsc_log_error(log, "compare values OK.");
    else
        smsc_log_error(log, "compare values: %d errors.", err);
    return err;
}



}
}

#endif
