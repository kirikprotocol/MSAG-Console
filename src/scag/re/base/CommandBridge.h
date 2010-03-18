#ifndef __SCAG_RULE_ENGINE_COMMAND_BRIDGE__
#define __SCAG_RULE_ENGINE_COMMAND_BRIDGE__

//#include "scag/transport/SCAGCommand.h"
#include "scag/transport/smpp/base/SmppCommand2.h"
#include "scag/exc/SCAGExceptions.h"
#include "scag/stat/base/Statistics2.h"
#include "scag/re/base/ActionContext2.h"
#include "scag/sessions/base/SessionKey.h"
#include "EventHandlerType.h"
// #include "CommandOperations.h"

/*namespace scag { namespace sessions {

struct CSessionPrimaryKey;

}}*/

namespace scag2 {
namespace re {

using namespace transport;
using namespace transport::smpp;
using namespace scag::exceptions;

using namespace stat;
using namespace smsc::smpp::UssdServiceOpValue;
using smsc::sms::Address;
//using actions::CommandProperty;

/*
struct CSmppDescriptor
{
    CommandOperations cmdType;
    int currentIndex;
    int lastIndex;

    bool isUSSDClosed;
    bool wantOpenUSSD;

    bool m_waitReceipt;
    bool isResp;

    CSmppDescriptor() : 
        currentIndex(0), 
        lastIndex(0), 
        isUSSDClosed(false), 
        wantOpenUSSD(false), 
        m_waitReceipt(false),
        isResp(false) {}
    
};
 */


void AssignAddress(Address& address, const char * str);


class CommandBridge
{
private:
    //static int getProtocolForEvent(SmppCommand& command);
    static EventHandlerType getSMPPHandlerType(const SCAGCommand& command);
    static EventHandlerType getHTTPHandlerType(const SCAGCommand& command);

public:
    static DataSmDirection getPacketDirection(const SCAGCommand& command);

    static bool hasMSB(const char* data, int len);
    static std::string getMessageBody(SmppCommand& command);
    //static void makeTrafficEvent(SmppCommand& command, int handlerType, scag::sessions::CSessionPrimaryKey& sessionPrimaryKey, SACC_TRAFFIC_INFO_EVENT_t& ev);
    static EventHandlerType getHandlerType(const SCAGCommand& command);
    // static CSmppDescriptor getSmppDescriptor(const SCAGCommand& command);

    static int getProtocolForEvent(SCAGCommand& command);


    static Address getDestAddr(const SCAGCommand& command)  
    {
        Address resultAddr;

        SCAGCommand& _command = const_cast<SCAGCommand&>(command);
        SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&_command);
        if (!smppCommand) throw SCAGException("Command Bridge Error: Cannot get DestAddress - SCAGCommand is not smpp-type");

        SMS& sms = getSMS(*smppCommand);
        return sms.getDestinationAddress();
    }

    static SMS& getSMS(SmppCommand& command)
    {
        SMS* sms = command.get_sms();
        if (!sms) throw SCAGException("Command Bridge Error: Cannot get SMS from SmppCommand");
        return *sms;
    }
/*
    static void setAbonentAddr(SmppCommand& command)
    {
        switch (command->get_smsCommand().dir)
        {
        case dsdSc2Srv:
            AssignAddress(sms->destinationAddress, property.getStr().c_str());
            break;
        case dsdSrv2Sc:
            AssignAddress(sms->originatingAddress, property.getStr().c_str());
            break;
        }
    }
*/
    static Address getAbonentAddr(SmppCommand& command)  
    {
        Address resultAddr;

        SMS& sms = getSMS(command);
        CommandId cmdid = CommandId(command.getCommandId());

        switch (cmdid) 
        {
        case DELIVERY:
            resultAddr = sms.originatingAddress;
            break;
        case SUBMIT:
            resultAddr = sms.destinationAddress;
            break;
        case DELIVERY_RESP:
            resultAddr = sms.originatingAddress;
            break;
        case SUBMIT_RESP:
            resultAddr = sms.destinationAddress;
            break;
        case DATASM:
            switch (command.get_smsCommand().dir)
            {
            case dsdSc2Srv:
                resultAddr = sms.originatingAddress;
                break;
            case dsdSrv2Sc:
                resultAddr = sms.destinationAddress;
                break;
            default:
                throw SCAGException("Command Bridge Error: wrong direction in datasm: %d", command.get_smsCommand().dir );
            }

            break;
        case DATASM_RESP:

            if (!command.get_resp()) break;

            switch (command.get_resp()->get_dir())
            {
            case dsdSc2Srv:
                resultAddr = sms.destinationAddress;
                break;
            case dsdSrv2Sc:
                resultAddr = sms.originatingAddress;
                break;
            default:
                throw SCAGException("Command Bridge Error: wrong direction in datasm: %d", command.get_resp()->get_dir() );
            }
            break;
        default:
            throw SCAGException("Command Bridge Error: Cannot get abonentAddress from SmppCommand");
        }
  
        return resultAddr;

    }



/*    static int16_t getUMR(const SCAGCommand& command)  
    {
//        Tag::SMPP_SAR_MSG_REF_NUM

        SCAGCommand& _command = const_cast<SCAGCommand&>(command);

        SmppCommand * smppCommand = dynamic_cast<SmppCommand *>(&_command);
        if (!smppCommand) throw SCAGException("Command Bridge Error: SCAGCommand is not smpp-type");


        SMS& sms = getSMS(*smppCommand);

        if (sms.hasIntProperty(Tag::SMPP_SAR_MSG_REF_NUM)) 
            return sms.getIntProperty(Tag::SMPP_SAR_MSG_REF_NUM);

        return 0;
    }*/

    static actions::CommandProperty getCommandProperty( SCAGCommand& command, const Address& abonentAddr, uint8_t operationType );
    static void CheckCommandProperty( SCAGCommand& command,
                                      const actions::CommandProperty& cp,
                                      const sessions::SessionPrimaryKey& primaryKey,
                                      sessions::Session* session);

    static void RegisterTrafficEvent( const actions::CommandProperty& commandProperty,
                                      const sessions::SessionPrimaryKey& sessionPrimaryKey,
                                      const std::string& messageBody,
                                      util::HRTiming* hrt = 0 );
    
    static void RegisterAlarmEvent( uint32_t eventId, const std::string& addr, uint8_t protocol,
                                    uint32_t serviceId, uint32_t providerId, uint32_t operatorId,
                                    uint16_t commandStatus, const sessions::SessionPrimaryKey& sessionPrimaryKey, char dir );
};


}}

#endif
