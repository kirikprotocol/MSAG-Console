#ident "$Id$"

#ifndef __SMSC_MTSMSME_PROCESSOR_SRI4SMTSM_HPP_
#define __SMSC_MTSMSME_PROCESSOR_SRI4SMTSM_HPP_

#include "TSM.hpp"
#include "Component.h"

namespace smsc{namespace mtsmsme{namespace processor{
/*
 * 17.2.2.23 Short message gateway services
 * This operation package includes the operations required for short message
 * service gateway procedures between MSC and HLR.
 * shortMsgGatewayPackage-v3 OPERATION-PACKAGE ::= {
 *   -- Supplier is HLR if Consumer is GMSC
 *   CONSUMER INVOKES {
 *     sendRoutingInfoForSM |
 *     reportSM-DeliveryStatus}
 *   SUPPLIER INVOKES {
 *     informServiceCentre} }
 * The v2-equivalent package can be determined according to
 * the rules described in clause 17.2.1.
 * The v1-equivalent package is defined as follows:
 * shortMsgGatewayPackage-v1 OPERATION-PACKAGE ::= {
 *   -- Supplier is HLR if Consumer is GMSC
 *   CONSUMER INVOKES {
 *     sendRoutingInfoForSM |
 *     reportSMDeliveryStatus} }
 * 17.3.2.21 Short Message Gateway
 * This application context is used for short message gateway procedures.
 * shortMsgGatewayContext-v3 APPLICATION-CONTEXT ::= {
 *   -- Responder is HLR if Initiator is GMSC
 *   INITIATOR CONSUMER OF {
 *     shortMsgGatewayPackage-v3}
 *   ID {map-ac shortMsgGateway(20) version3(3)} }
 * The following application-context-name is assigned to
 * the v2-equivalent application-context:
 * ID {map-ac shortMsgGateway(20) version2(2)}
 * The following application-context-name is assigned to
 * the v1-equivalent application-context:
 * ID {map-ac shortMsgGateway(20) version1(1)}
 */
class SRI4SMTSM : public TSM
{
  public:
    SRI4SMTSM(TrId _ltrid,AC& ac,TCO* _tco);
    SRI4SMTSM(TrId _ltrid,AC& ac,TCO* _tco, const char* _imsi, const char* _msc,const char* _vlr, const char* _mgt);
    virtual ~SRI4SMTSM();
    virtual void BEGIN(Message& msg);
    virtual void CONTINUE_received(uint8_t cdlen,
                                   uint8_t *cd, /* called party address */
                                   uint8_t cllen,
                                   uint8_t *cl, /* calling party address */
                                   Message& msg);

    virtual void TBeginReq(uint8_t  cdlen,
                           uint8_t* cd,        /* called party address */
                           uint8_t  cllen,
                           uint8_t* cl        /* calling party address */);

    virtual void TInvokeReq(int8_t invokeId, uint8_t opcode, CompIF& arg);
    virtual void TEndReq();
    virtual void TResultLReq(uint8_t invokeId, uint8_t opcode, CompIF& arg);
    virtual void TUErrorReq(int invokeId, uint8_t errcode, CompIF& arg);
  private:
    string imsi;
    string msc;
    string vlr;
    string mgt;
    vector<unsigned char> temp_arg;
    uint8_t temp_opcode;
    uint8_t temp_errcode;
    int8_t temp_invokeId;
};

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
#endif //__SMSC_MTSMSME_PROCESSOR_SRI4SMTSM_HPP_
