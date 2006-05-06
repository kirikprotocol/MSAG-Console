#ifndef SCAG_TRANSPORT_SMPP_RECEIPT_MANAGER
#define SCAG_TRANSPORT_SMPP_RECEIPT_MANAGER

#include <string>

#include "scag/transport/SCAGCommand.h"
#include "scag/sessions/Session.h"
#include "sms/sms.h"

namespace scag { namespace sessions 
{
    using scag::transport::SCAGCommand;
    using smsc::sms::Address;
    using scag::sessions::CSessionKey;

    class ReceiptManager 
    {
        ReceiptManager (const ReceiptManager& sm);
        ReceiptManager& operator=(const ReceiptManager& sm);
    protected:
        virtual ~ReceiptManager () {};
        ReceiptManager() {};
    public:

        static void Init(std::string& storePath);
        static ReceiptManager& Instance();

        virtual void waitReceipt(Address& service, CSessionKey& sessionKey) = 0;
        virtual void cancelReceipt(Address& service, CSessionKey& sessionKey) = 0;
        virtual int updateReceipt(Address& service, CSessionKey& sessionKey, std::string& smsc_msg_id, SCAGCommand& receiptCommand) = 0;
        virtual int registerReceipt(Address& service, CSessionKey& sessionKey, std::string& smsc_msg_id, SCAGCommand& receiptCommand) = 0;

    };
}}

#endif 
