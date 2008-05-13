#ident "$Id$"
#ifndef SMSC_MTSM_SME_PRECESSOR
#define SMSC_MTSM_SME_PRECESSOR

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <sms/sms.h>

#include <sms/sms.h>
//#include <util/recoder/recode_dll.h>
//#include <util/smstext.h>

namespace smsc { namespace mtsmsme { namespace processor
{

    using namespace smsc::sms;
    using smsc::sms::Address;
    /**
     * Interface & sms to send
     * Note: Need to be implemnted by lower layer.
     */
    struct Request
    {
        SMS sms;

        /**
         * Callback function for setting result of send operation (responce code)
         * Note: Callback function need to be implemnted by lower layer.
         *
         * @param   result      SMSC responce code
         */
        virtual void setSendResult(int result) = 0;
    };

    /**
     * Interface for messages sending.
     * Note: Need to be implemnted by upper layer.
     */
    class RequestSender
    {
    public:

        /**
         * Sends data contained in request via SMPP to SMSC.
         * Note: Need to be implemnted by upper layer
         *
         * @param   request     request to send
         * @return  true if request was sent, false otherwise
         */
        virtual bool send(Request* request) = 0;

        virtual ~RequestSender() {};

    protected:

        RequestSender() {};
    };

    /**
     * Interface for HLR operation and maintenance command
     */
   class HLROAM
   {
     public:
       // request to register specified info to HLR on periodical basis specified by period.
       // if 'period' parameter equals zero then register info only once 
       virtual void registerSubscriber(Address& imsi, Address& msisdn, Address& mgt, int period) = 0;
   };
    /**
     * Interface for request manager
     * Note: Need to be implemnted by lower layer.
     */
    class RequestProcessor
    {
    public:

        static RequestProcessor* getInstance();
        virtual void configure(int user_id, int ssn, Address& msc, Address& vlr, Address& hlr) = 0;
        virtual void setRequestSender(RequestSender* sender) = 0;
        virtual HLROAM* getHLROAM() = 0;

        virtual int Run() = 0;
        virtual void Stop() = 0;

        virtual ~RequestProcessor() {};

    protected:

        RequestProcessor() {};
    };
}}}

#endif // SMSC_MTSM_SME_PRECESSOR
