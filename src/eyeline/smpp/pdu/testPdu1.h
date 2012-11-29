#ifndef _EYELINE_SMPP_TESTPDU1_H
#define _EYELINE_SMPP_TESTPDU1_H

#include "testPdu.h"
#include "DeliverSmPdu.h"
//#include "SubmitSmPdu.h"
//#include "TlvDefines.h"

namespace eyeline {
namespace smpp {

class DeliverSmPduContainer : public PduContainer
{
public:
    DeliverSmPduContainer(const uint32_t int_value, const char* str_value, smsc::logger::Logger* logptr)
        : PduContainer(int_value, str_value, logptr)
    {
        dp = new DeliverSmPdu();
        pdu = dp;
    }
    DeliverSmPduContainer(DeliverSmPdu* newpdu, smsc::logger::Logger* logptr)
        : PduContainer(1, "", logptr)
    {
        dp = newpdu;
        pdu = dp;
    }
    ~DeliverSmPduContainer() { pdu = 0; delete dp; }

protected:
    DeliverSmPdu* dp;

protected:
    int fillMandatory() {
        //mandatory int
        dp->setDataCoding(value);
        dp->setEsmClass(value);
        dp->setPriorityFlag(value);
        dp->setProtocolId(value);
        dp->setRegisteredDelivery(value);
        dp->setReplaceIfPresentFlag(value);
        dp->setSmDefaultMsgId(value);
        //mandatory string
        dp->setScheduleDeliveryTime(buf);
        dp->setServiceType(buf);
        dp->setValidityPeriod(buf);
        //complex
        dp->setDestAddr(value, value, buf);     //npi,ton addr
        dp->setSourceAddr(value, value, buf);
        dp->setShortMessage(buf, strlen(buf));  //SmLength
        return 1;
    }
    int logMandatory() {
        //mandatory int
        smsc_log_info(log, "DataCoding %d", dp->getDataCoding());
        smsc_log_info(log, "DestAddrNpi %d", dp->getDestAddrNpi());
        smsc_log_info(log, "DestAddrTon %d", dp->getDestAddrTon());
        smsc_log_info(log, "EsmClass %d", dp->getEsmClass());
        smsc_log_info(log, "PriorityFlag %d", dp->getPriorityFlag());
        smsc_log_info(log, "ProtocolId %d", dp->getProtocolId());
        smsc_log_info(log, "RegisteredDelivery %d", dp->getRegisteredDelivery());
        smsc_log_info(log, "ReplaceIfPresentFlag %d", dp->getReplaceIfPresentFlag());
        smsc_log_info(log, "SmDefaultMsgId %d", dp->getSmDefaultMsgId());
        smsc_log_info(log, "SmLength %d", dp->getSmLength());
        smsc_log_info(log, "SourceAddrNpi %d", dp->getSourceAddrNpi());
        smsc_log_info(log, "SourceAddrTon %d", dp->getSourceAddrTon());
        //mandatory string
        smsc_log_info(log, "DestAddr %s", dp->getDestAddr());
        smsc_log_info(log, "ScheduleDeliveryTime %s", dp->getScheduleDeliveryTime());
        smsc_log_info(log, "ServiceType %s", dp->getServiceType());
        smsc_log_info(log, "ShortMessage %s", dp->getShortMessage());
        smsc_log_info(log, "SourceAddr %s", dp->getSourceAddr());
        smsc_log_info(log, "ValidityPeriod %s", dp->getValidityPeriod());
        return 1;
    }
#define CHECK_TLV_STRING(x) if (strcmp(dp->x(), compDp->x()) != 0) ++result
#define CHECK_TLV_INT(x) if (dp->x() != compDp->x()) ++result
    int compareMandatory(Pdu* compPdu) {
        int result = 0;
        DeliverSmPdu* compDp = reinterpret_cast<DeliverSmPdu*>(compPdu);
        //mandatory int
        CHECK_TLV_INT(getDataCoding);
        CHECK_TLV_INT(getDestAddrNpi);
        CHECK_TLV_INT(getDestAddrTon);
        CHECK_TLV_INT(getEsmClass);
        CHECK_TLV_INT(getPriorityFlag);
        CHECK_TLV_INT(getProtocolId);
        CHECK_TLV_INT(getRegisteredDelivery);
        CHECK_TLV_INT(getReplaceIfPresentFlag);
        CHECK_TLV_INT(getSmDefaultMsgId);
        CHECK_TLV_INT(getSmLength);
        CHECK_TLV_INT(getSourceAddrNpi);
        CHECK_TLV_INT(getSourceAddrTon);
        //mandatory string
        CHECK_TLV_STRING(getDestAddr);
        CHECK_TLV_STRING(getScheduleDeliveryTime);
        CHECK_TLV_STRING(getServiceType);
        CHECK_TLV_STRING(getShortMessage);
        CHECK_TLV_STRING(getSourceAddr);
        CHECK_TLV_STRING(getValidityPeriod);
        return result;
    }
#undef CHECK_TLV_INT
#undef CHECK_TLV_STRING
};


int test1(const uint32_t int_value, const char* str_value, smsc::logger::Logger* log) {
    DeliverSmPduContainer d1(int_value, str_value, log);

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

    DeliverSmPduContainer d2(reinterpret_cast<DeliverSmPdu*>(pdu), log);
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
